#include "scene.h"
#include "lvgl.h"
#include "canvas.h"
#include "clock.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "data_fetcher.h"
#include "currency.h"
#include <stdatomic.h>
#include <stdbool.h>
#define MAIN_SCENE_DURATION_SEC 5

static const char *TAG = "Scene";
static lv_obj_t *canvas_obj;
static lv_obj_t *clock_obj;
static lv_obj_t *currency_obj;

static atomic_int current_scene = SCENE_CLOCK;
static atomic_bool is_main_scene = true;

void scene_set(scene_t scene)
{
    ESP_LOGI(TAG, "current scene: %d set to: %d", current_scene, scene);

    switch (scene)
    {
    case SCENE_CLOCK:
        atomic_store(&is_main_scene, false);
        break;
    case SCENE_CANVAS:
        atomic_store(&is_main_scene, false);
        break;
    case SCENE_CURRENCY:
        atomic_store(&is_main_scene, false);
        break;
    case SCENE_MAIN:
        atomic_store(&is_main_scene, true);
    default:
        break;
    }

    atomic_store(&current_scene, (int)scene);
}
static void main_scene_task(void *arg)
{
    static uint32_t data_update_ticker = 20;
    while (1)
    {
        vTaskDelay((MAIN_SCENE_DURATION_SEC * 1000) / portTICK_PERIOD_MS);
        if (data_update_ticker < 1)
        {
            data_update_ticker = 60 * 60 * 12;
            fetch_data_t data = {0};
            get_fetch_data(&data);
            ESP_LOGI(TAG, "data update");
            currency_update(&data);
        }
        data_update_ticker = data_update_ticker - MAIN_SCENE_DURATION_SEC;
        ESP_LOGI(TAG, "data update counter: %d", data_update_ticker);

        if (!is_main_scene)
        {
            continue;
        }
        switch (current_scene)
        {
        case SCENE_CLOCK:
            scene_set(SCENE_CURRENCY);
            break;
        case SCENE_CURRENCY:
            scene_set(SCENE_CLOCK);
            break;
        case SCENE_CANVAS:
            break;
        default:
            scene_set(SCENE_CLOCK);
            break;
        }
        atomic_store(&is_main_scene, true);
    }
}

static void scene_switcher_cb(lv_timer_t *timer)
{
    scene_t scene = (scene_t)atomic_load(&current_scene);

    ESP_LOGI(TAG, "Switching display to scene ID: %d", scene);

    switch (scene)
    {
    case SCENE_CLOCK:
        lv_scr_load(clock_obj);
    case SCENE_CURRENCY:
        lv_scr_load(currency_obj);
        break;
    case SCENE_CANVAS:
        lv_scr_load(canvas_obj);
        break;
    default:
        break;
    }
}

void scene_init()
{
    canvas_obj = canvas_init();
    currency_obj = currency_init();
    clock_obj = clock_init();
    scene_set(SCENE_MAIN);
    xTaskCreate(main_scene_task, "MainScene", 4096, NULL, 5, NULL);
    lv_timer_t *timer = lv_timer_create(scene_switcher_cb, 300, NULL);

    ESP_LOGI(TAG, "initalized");
}
