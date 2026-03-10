
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
    keypad_init();

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

    scene_event(SCENE_SET_CHIP8, NULL);

    // while (1)
    // {
    //     keypad_key_t key = keypad_scan();

    //     if (key != KEY_NONE)
    //     {
    //         if (key == KEY_1)
    //         {
    //             printf("Pressed 1\n");
    //             scene_event(SCENE_SET_CHIP8, NULL);
    //         }

    //         if (key == KEY_HASH)
    //         {
    //             scene_event(SCENE_SET_MAIN, NULL);
    //             printf("# pressed\n");
    //         }
    //     }

    //     vTaskDelay(pdMS_TO_TICKS(100));
    // }
}
