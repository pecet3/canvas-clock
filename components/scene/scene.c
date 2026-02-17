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

#define MAIN_SCENE_DURATION_SEC 5

static const char *TAG = "Scene";
static lv_obj_t *canvas_obj;
static lv_obj_t *clock_obj;
static lv_obj_t *currency_obj;

static scene_t current_scene = SCENE_CLOCK;
static bool is_main_scene = true;

void scene_set(scene_t scene)
{
    ESP_LOGI(TAG, "current scene: %d set to: %d", current_scene, scene);

    display_mux_lock();

    lv_obj_add_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(canvas_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(currency_obj, LV_OBJ_FLAG_HIDDEN);
    switch (scene)
    {
    case SCENE_CLOCK:
        lv_obj_clear_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
        is_main_scene = false;
        break;
    case SCENE_CANVAS:
        lv_obj_clear_flag(canvas_obj, LV_OBJ_FLAG_HIDDEN);
        canvas_fill_color(0);
        is_main_scene = false;
        break;
    case SCENE_CURRENCY:
        lv_obj_clear_flag(currency_obj, LV_OBJ_FLAG_HIDDEN);
        is_main_scene = false;
        break;
    case SCENE_MAIN:
        lv_obj_clear_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
        is_main_scene = true;
    default:
        break;
    }

    current_scene = scene;

    display_mux_unlock();
}
static void main_scene_task(void *arg)
{
    static uint32_t data_update_timer = 20;
    while (1)
    {
        vTaskDelay((MAIN_SCENE_DURATION_SEC * 1000) / portTICK_PERIOD_MS);
        if (data_update_timer < 1)
        {
            data_update_timer = 60 * 60 * 12;
            fetch_data_t data = {0};
            get_fetch_data(&data);
            ESP_LOGI(TAG, "data update");
            currency_update(&data);
        }
        data_update_timer = data_update_timer - MAIN_SCENE_DURATION_SEC;
        ESP_LOGI(TAG, "data update counter: %d", data_update_timer);

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
        display_mux_lock();
        is_main_scene = true;
        display_mux_unlock();
    }
}

void scene_init()
{
    canvas_obj = canvas_init();
    currency_obj = currency_init();
    clock_obj = clock_init();

    xTaskCreate(main_scene_task, "MainScene", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "initalized");
}
