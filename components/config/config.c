#include "config.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/err.h"
#include "cJSON.h"
#include <stdatomic.h>
#include <stdint.h>

#define CONFIG_PATH "/spiffs/config.bin"

static config_t config = {0};
static SemaphoreHandle_t cfg_mutex;

static const char *TAG = "config";

static bool save_cfg_file()
{
    FILE *file = fopen(CONFIG_PATH, "wb");
    if (file == NULL)
    {
        return false;
    }
    size_t n = fwrite(&config, 1, sizeof(config), file);
    if (n != sizeof(config))
    {
        fclose(file);
        ESP_LOGE(TAG, "Failed to save file");
        return false;
    }
    fflush(file);
    fclose(file);
    return true;
}

static bool load_cfg_file()
{
    FILE *file = fopen(CONFIG_PATH, "rb");

    if (file == NULL)
    {
        ESP_LOGE(TAG, "No saved config found");
        return false;
    }
    fread(&config, 1, sizeof(config), file);
    fclose(file);
    ESP_LOGI(TAG, "config loaded from SPIFFS");
    return true;
}

/*
    PUBLIC
*/
void config_get(config_t *cfg)
{
    if (xSemaphoreTake(cfg_mutex, portMAX_DELAY) == pdTRUE)
    {
        memcpy(cfg, &config, sizeof(config));
        xSemaphoreGive(cfg_mutex);
    }
}

bool config_set(config_t *cfg)
{
    if (xSemaphoreTake(cfg_mutex, portMAX_DELAY) == pdTRUE)
    {
        memcpy(&config, cfg, sizeof(cfg));
        xSemaphoreGive(cfg_mutex);
        return true;
    }
    return false;
}

void config_init()
{
    bool ok = load_cfg_file();
    if (!ok)
    {
        save_cfg_file();
    }
    cfg_mutex = xSemaphoreCreateMutex();
}