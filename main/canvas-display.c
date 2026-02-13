
#include "wifi.h"
#include "display.h"
#include "captive_portal.h"
#include "data_fetcher.h"
#include "scene.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void)
{
    display_init();
    wifi_init();
    vTaskDelay(pdMS_TO_TICKS(1000));
    scene_init();
    captive_portal_init();
    data_fetcher_init();
}
