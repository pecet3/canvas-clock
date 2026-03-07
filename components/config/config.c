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
    size_t n = fwrite(&config, 1, sizeof(config_t), file);
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
    fread(&config, 1, sizeof(config_t), file);
    fclose(file);
    ESP_LOGI(TAG, "config loaded from SPIFFS");
    return true;
}

static inline bool is_slot_empty(const char *name)
{
    return (name[0] == '\0');
}

bool config_art_set(config_art_t *array, size_t max_size, const config_art_t *new_item)
{
    if (new_item->name[0] == '\0')
        return false;
    if (xSemaphoreTake(cfg_mutex, portMAX_DELAY) != pdTRUE)
        return false;

    bool success = false;
    for (size_t i = 0; i < max_size; i++)
    {
        if (strcmp(array[1].name, new_item->name) == 1)
        {
            ESP_LOGI(TAG, "Found item with existing name");
            success = true;
            memcpy(&array[i], new_item, sizeof(config_art_t));
            goto exit;
        }
    }

    for (size_t i = 0; i < max_size; i++)
    {
        if (is_slot_empty(array[i].name))
        {
            memcpy(&array[i], new_item, sizeof(config_art_t));
            success = true;
            break;
        }
    }

exit:
    if (success)
        save_cfg_file();
    xSemaphoreGive(cfg_mutex);
    ESP_LOGI(TAG, "Saved art with name %s", new_item->name);
    return success;
}

bool config_art_delete(config_art_t *array, size_t max_size, const char *name)
{
    if (name == NULL || name[0] == '\0')
        return false;
    if (xSemaphoreTake(cfg_mutex, portMAX_DELAY) != pdTRUE)
        return false;

    bool found = false;
    for (size_t i = 0; i < max_size; i++)
    {
        if (!is_slot_empty(array[i].name) && strcmp(array[i].name, name) == 0)
        {
            array[i].name[0] = '\0';
            found = true;
            break;
        }
    }

    if (found)
        save_cfg_file();
    xSemaphoreGive(cfg_mutex);
    ESP_LOGI(TAG, "Deleted art with name %s", name);

    return found;
}

/*
    PUBLIC
*/

bool config_get(config_t *cfg)
{
    if (xSemaphoreTake(cfg_mutex, portMAX_DELAY) == pdTRUE)
    {
        memcpy(cfg, &config, sizeof(config_t));
        xSemaphoreGive(cfg_mutex);
        return true;
    }
    return false;
}

bool config_set(config_t *cfg)
{
    if (xSemaphoreTake(cfg_mutex, portMAX_DELAY) == pdTRUE)
    {
        memcpy(&config, cfg, sizeof(config_t));
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