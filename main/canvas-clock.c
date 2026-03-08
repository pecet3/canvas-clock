
#include "wifi.h"
#include "display.h"
#include "captive_portal.h"
#include "data_fetcher.h"
#include "scene.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "buzzer.h"
#include "alarm.h"
#include "keypad.h"
#include "storage.h"
void app_main(void)
{
    buzzer_init();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    storage_init();
    display_init();
    scene_init();
    wifi_init();
    captive_portal_init();
    data_fetcher_init();
    alarm_init();
    keypad_init();

    while (1)
    {
        char key = keypad_scan();

        if (key)
        {
            printf("Pressed: %c\n", key);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
