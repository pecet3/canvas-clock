#include "esp_spiffs.h"
#include <stdio.h>

void spiffs_init()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true};

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