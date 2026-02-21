/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"
#include "data_fetcher.h"

#define WEB_SERVER "api.nbp.pl"
#define WEB_PORT "80"
#define WEB_PATH "/api/exchangerates/tables/a?format=json"

static const char *TAG = "http request";

static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER ":" WEB_PORT "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

static fetch_data_t fetched_data = {0};
static SemaphoreHandle_t data_mutex;

void set_time_from_http_header(const char *date_str)
{
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    if (strptime(date_str, "%a, %d %b %Y %H:%M:%S", &tm) == NULL)
    {
        ESP_LOGE(TAG, "Date parsing error: %s", date_str);
        return;
    }

    time_t t = mktime(&tm);

    struct timeval now = {.tv_sec = t + 3600};
    settimeofday(&now, NULL);

    ESP_LOGI(TAG, "Date is set from http header");
}

bool get_fetch_data(fetch_data_t *data)
{
    if (data == NULL)
    {
        return false;
    }
    if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE)
    {
        if (fetched_data.usd_mid == 0 && fetched_data.eur_mid == 0 && fetched_data.gbp_mid == 0 && fetched_data.czk_mid == 0)
        {
            xSemaphoreGive(data_mutex);
            return false;
        }
        memcpy(data, &fetched_data, sizeof(fetch_data_t));
        xSemaphoreGive(data_mutex);
        return true;
    }
    return false;
}

static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;

    while (1)
    {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if (err != 0 || res == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0)
        {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
        {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0)
        {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                       sizeof(receiving_timeout)) < 0)
        {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        char *resp_buf = malloc(4096);
        if (!resp_buf)
        {
            ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        size_t total_len = 0;
        do
        {
            r = read(s, resp_buf + total_len, 4096 - total_len);
            if (r > 0)
            {
                total_len += r;
                resp_buf[total_len] = 0; // Null-terminate the buffer
            }
        } while (r > 0 && total_len < 4096);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);

        char *line = strstr(resp_buf, "Date: ");
        if (line)
        {
            line += 6;
            char *end_of_line = strstr(line, "\r\n");
            if (end_of_line)
            {
                *end_of_line = '\0';
                set_time_from_http_header(line);
                *end_of_line = '\r';
            }
        }

        // json
        char *json_body = strstr(resp_buf, "\r\n\r\n");
        if (json_body == NULL)
        {
            goto cleanup_and_wait;
        }
        json_body += 4;
        ESP_LOGI(TAG, "JSON Body: %s", json_body);

        cJSON *json = cJSON_Parse(json_body);
        if (json == NULL)
        {
            ESP_LOGE(TAG, "Failed to parse JSON");
            goto cleanup_and_wait;
        }
        cJSON *escaped_arr_json = cJSON_GetArrayItem(json, 0);
        if (escaped_arr_json == NULL)
        {
            ESP_LOGE(TAG, "Failed to get first array item from JSON");
            cJSON_Delete(json);
            goto cleanup_and_wait;
        }
        cJSON *rates = cJSON_GetObjectItem(escaped_arr_json, "rates");
        if (rates == NULL)
        {
            ESP_LOGE(TAG, "Failed to get 'rates' from JSON");
            cJSON_Delete(json);

            goto cleanup_and_wait;
        }
        double usd_mid = 0, eur_mid = 0, gbp_mid = 0, czk_mid = 0;

        for (int i = 0; i < cJSON_GetArraySize(rates); i++)
        {
            cJSON *rate = cJSON_GetArrayItem(rates, i);
            cJSON *code = cJSON_GetObjectItem(rate, "code");
            cJSON *mid = cJSON_GetObjectItem(rate, "mid");
            if (code != NULL && mid != NULL)
            {
                ESP_LOGI(TAG, "Currency: %s, Rate: %.4f", code->valuestring, mid->valuedouble);
                if (strcmp(code->valuestring, "USD") == 0)
                {
                    usd_mid = mid->valuedouble;
                }
                else if (strcmp(code->valuestring, "EUR") == 0)
                {
                    eur_mid = mid->valuedouble;
                }
                else if (strcmp(code->valuestring, "GBP") == 0)
                {
                    gbp_mid = mid->valuedouble;
                }
                else if (strcmp(code->valuestring, "JPY") == 0)
                {
                    czk_mid = mid->valuedouble;
                }
            }
        }

        if (xSemaphoreTake(data_mutex, portMAX_DELAY) == pdTRUE)
        {
            fetched_data.usd_mid = usd_mid;
            fetched_data.eur_mid = eur_mid;
            fetched_data.gbp_mid = gbp_mid;
            fetched_data.czk_mid = czk_mid;
            xSemaphoreGive(data_mutex);
        }

        cJSON_Delete(json);

    cleanup_and_wait:
        close(s);
        free(resp_buf);
        // wait 12h to next fetch
        vTaskDelay(1000 * 60 * 60 * 12 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "Starting again!");
    }
}

void data_fetcher_init(void)
{
    data_mutex = xSemaphoreCreateMutex();
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}