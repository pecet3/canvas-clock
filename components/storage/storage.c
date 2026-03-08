#include "storage.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lwip/err.h"
#include <stdatomic.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_spiffs.h"
#include <stdio.h>

#define STORAGE_FILE "/spiffs/storage"
#define STORAGE_MAX_FILES 1 + 64 * 2

static storage_t storage = {0};
static SemaphoreHandle_t storage_mux;

static const char *TAG = "Storage";

static void spiffs_init()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = STORAGE_MAX_FILES,
        .format_if_mount_failed = true,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            printf("Failed to mount or format SPIFFS\n");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            printf("SPIFFS partition not found\n");
        }
        else
        {
            printf("Failed to initialize SPIFFS (%d)\n", ret);
        }
    }
    else
    {
        size_t total = 0, used = 0;
        esp_spiffs_info("storage", &total, &used);
        printf("SPIFFS mounted. Total: %d, Used: %d\n", total, used);
    }
}
static bool save_storage()
{
    FILE *file = fopen(STORAGE_FILE, "wb");
    if (file == NULL)
    {
        return false;
    }
    size_t n = fwrite(&storage, 1, sizeof(storage_t), file);
    if (n != sizeof(storage_t))
    {
        fclose(file);
        ESP_LOGE(TAG, "Failed to save file");
        return false;
    }
    fflush(file);
    fclose(file);
    return true;
}

static bool load_storage()
{
    FILE *file = fopen(STORAGE_FILE, "rb");

    if (file == NULL)
    {
        ESP_LOGE(TAG, "No saved  found");
        return false;
    }
    fread(&storage, 1, sizeof(storage_t), file);
    fclose(file);
    ESP_LOGI(TAG, " loaded from SPIFFS");
    return true;
}

static inline bool is_entity_empty(const char *name)
{
    return (name[0] == '\0');
}

static bool art_set(storage_art_t *array, size_t max_size, const storage_art_t *new_item)
{
    if (new_item->name[0] == '\0')
        return false;
    if (xSemaphoreTake(storage_mux, portMAX_DELAY) != pdTRUE)
        return false;

    bool success = false;
    for (size_t i = 0; i < max_size; i++)
    {
        if (strcmp(array[1].name, new_item->name) == 1)
        {
            ESP_LOGI(TAG, "Found item with existing name");
            success = true;
            memcpy(&array[i], new_item, sizeof(storage_art_t));
            goto exit;
        }
    }

    for (size_t i = 0; i < max_size; i++)
    {
        if (is_entity_empty(array[i].name))
        {
            memcpy(&array[i], new_item, sizeof(storage_art_t));
            success = true;
            break;
        }
    }

exit:
    if (success)
        save_storage();
    xSemaphoreGive(storage_mux);
    ESP_LOGI(TAG, "Saved entity with name %s", new_item->name);
    return success;
}

static bool art_delete(storage_art_t *array, size_t max_size, const char *name)
{
    if (name == NULL || name[0] == '\0')
        return false;
    if (xSemaphoreTake(storage_mux, portMAX_DELAY) != pdTRUE)
        return false;

    bool found = false;
    for (size_t i = 0; i < max_size; i++)
    {
        if (!is_entity_empty(array[i].name) && strcmp(array[i].name, name) == 0)
        {
            memset(&array[i], 0, sizeof(storage_art_t));
            found = true;
            break;
        }
    }

    if (found)
        save_storage();
    xSemaphoreGive(storage_mux);
    ESP_LOGI(TAG, "Deleted entity with name %s", name);

    return found;
}

/*
    PUBLIC
*/
bool storage_art_set(storage_art_kind_t kind, const char *name)
{
    storage_art_t item = {0};
    item.is_enabled = true;
    item.created_at = 0;
    strncpy(item.name, name, sizeof(item.name) - 1);
    item.name[sizeof(item.name) - 1] = '\0';

    switch (kind)
    {
    case STORAGE_ART_PAINTING:
    {
        return art_set(storage.paintings, 64, &item);
    }
    case STORAGE_ART_MELODY:
        return art_set(storage.melodies, 64, &item);
    }
    return false;
}
bool storage_art_delete(storage_art_kind_t kind, const char *name)
{
    switch (kind)
    {
    case STORAGE_ART_PAINTING:
    {
        return art_delete(storage.paintings, 64, name);
    }
    case STORAGE_ART_MELODY:
        return art_delete(storage.melodies, 64, name);
    }
    return false;
}

void storage_mux_lock()
{
    xSemaphoreTake(storage_mux, portMAX_DELAY);
}
void storage_mux_unlock()
{
    xSemaphoreGive(storage_mux);
}

storage_t *storage_ptr()
{
    return &storage;
}

void storage_init()
{
    spiffs_init();
    bool ok = load_storage();
    if (!ok)
    {
        save_storage();
    }
    storage_mux = xSemaphoreCreateMutex();
}