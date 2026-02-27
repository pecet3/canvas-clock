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

static const char *TAG = "Scene";
static lv_obj_t *canvas_obj;
static lv_obj_t *clock_obj;
static lv_obj_t *currency_obj;

static scene_t current_scene = SCENE_CLOCK;
static bool is_autocycle = true;

extern bool canvas_load_slot_locked(const char *nvs_key);
extern const char *canvas_get_nvs_key(int num);
extern void canvas_fill_color_locked(uint32_t color);
extern void canvas_set_drawing_locked();
extern void canvas_set_showing_locked();

static void scene_set_locked(scene_t scene)
{
    ESP_LOGI(TAG, "current scene: %d set to: %d", current_scene, scene);
    lv_obj_add_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(canvas_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(currency_obj, LV_OBJ_FLAG_HIDDEN);
    switch (scene)
    {
    case SCENE_CLOCK:
        lv_obj_clear_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
        is_autocycle = false;
        break;
    case SCENE_CANVAS_DRAW:
        lv_obj_clear_flag(canvas_obj, LV_OBJ_FLAG_HIDDEN);
        canvas_set_drawing_locked();
        is_autocycle = false;
        break;
    case SCENE_CANVAS_SHOW:
        lv_obj_clear_flag(canvas_obj, LV_OBJ_FLAG_HIDDEN);
        canvas_set_showing_locked();
        is_autocycle = false;
        break;
    case SCENE_CURRENCY:
        lv_obj_clear_flag(currency_obj, LV_OBJ_FLAG_HIDDEN);
        is_autocycle = false;
        break;
    case SCENE_MAIN:
        lv_obj_clear_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
        is_autocycle = true;
        scene = SCENE_CLOCK;
        break;
    default:
        break;
    }
    current_scene = scene;
}

static void main_scene_task(void *arg)
{
    // wait for init other depedencies like wifi or data_fetcher
    vTaskDelay((20 * 1000) / portTICK_PERIOD_MS);
    static int data_update_ticker = 0;
    while (1)
    {
        if (data_update_ticker < 1)
        {
            fetch_data_t data = {0};
            bool ok = get_fetch_data(&data);
            if (ok)
            {
                ESP_LOGI(TAG, "data update");
                currency_update(&data);
                data_update_ticker = 100;
            }
            else
            {
                ESP_LOGI(TAG, "data not ready");
                data_update_ticker = 0;
            }
        }
        --data_update_ticker;

        ESP_LOGI(TAG, "data update counter: %d", data_update_ticker);

        if (!is_autocycle)
        {
            vTaskDelay((10 * 1000) / portTICK_PERIOD_MS);
            continue;
        }
        display_mux_lock();
        switch (current_scene)
        {
        case SCENE_CLOCK:
            scene_set_locked(SCENE_CURRENCY);
            break;
        case SCENE_CURRENCY:
            scene_set_locked(SCENE_CANVAS_SHOW);
            break;
        case SCENE_CANVAS_SHOW:
            scene_set_locked(SCENE_CLOCK);
            break;
        default:
            scene_set_locked(SCENE_CLOCK);
            break;
        }
        is_autocycle = true;
        display_mux_unlock();

        switch (current_scene)
        {
        case SCENE_CLOCK:
            vTaskDelay((12 * 1000) / portTICK_PERIOD_MS);
            break;
        case SCENE_CURRENCY:
            vTaskDelay((6 * 1000) / portTICK_PERIOD_MS);
            break;
        case SCENE_CANVAS_SHOW:
            vTaskDelay((8 * 1000) / portTICK_PERIOD_MS);
            break;
        default:
            vTaskDelay((6 * 1000) / portTICK_PERIOD_MS);
            break;
        }
    }
}

void scene_set(scene_t scene)
{
    ESP_LOGI(TAG, "current scene: %d set to: %d", current_scene, scene);

    display_mux_lock();

    scene_set_locked(scene);
    display_mux_unlock();
}

void scene_init()
{
    canvas_obj = canvas_init();
    currency_obj = currency_init();
    clock_obj = clock_init();

    xTaskCreate(main_scene_task, "MainScene", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "initalized");
}
