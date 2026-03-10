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
#include "storage.h"
#include "chip8.h"
extern bool canvas_load_slot_locked(const char *nvs_key);
extern const char *canvas_get_nvs_key(int num);
extern void canvas_fill_color_locked(uint32_t color);
extern void canvas_set_drawing_locked();
extern bool canvas_set_showing_locked();

static const char *TAG = "Scene";

static lv_obj_t *canvas_obj;
static lv_obj_t *clock_obj;
static lv_obj_t *currency_obj;

typedef enum
{
    SCENE_CLOCK,
    SCENE_CANVAS_DRAW,
    SCENE_CANVAS_SHOW,
    SCENE_CANVAS_ALARM,
    SCENE_CURRENCY,
    SCENE_MAIN,
} scene_t;

const static scene_t default_main_scene = SCENE_CLOCK;
static scene_t current_scene = default_main_scene;

static bool is_autocycle = true;
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
        if (!canvas_set_showing_locked())
        {
            scene_set_locked(SCENE_MAIN);
            is_autocycle = true;
            break;
        }
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
        data_update_ticker--;

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

void scene_event(scene_event_t event, void *data)
{
    ESP_LOGI(TAG, "event: %d", event);
    display_mux_lock();
    switch (event)
    {
    case SCENE_SET_MAIN:
    {
        scene_set_locked(SCENE_MAIN);
        chip8_stop();
        break;
    }

    case SCENE_SET_CANVAS:
    {
        scene_set_locked(SCENE_CANVAS_DRAW);
        is_autocycle = false;
        break;
    }

    case SCENE_SET_CHIP8:
    {
        scene_set_locked(SCENE_CANVAS_DRAW);
        chip8_run();
        break;
    }

    case SCENE_CANVAS_SAVE_SLOT:
    {
        if (data == NULL)
            break;

        const char *name = (const char *)data;
        if (canvas_save_slot_locked(name))
        {
            ESP_LOGI(TAG, "saved ");
        }
        bool ok = storage_art_set(STORAGE_ART_PAINTING, name);
        if (ok)
        {
            ESP_LOGI(TAG, "saved ");
        }
        break;
    }
    case SCENE_CANVAS_LOAD_SLOT:
    {
        if (data == NULL)
            break;

        const char *name = (const char *)data;
        canvas_load_slot_locked(name);
        break;
    }
    case SCENE_CANVAS_DELETE_SLOT:
    {
        if (data == NULL)
            break;

        const char *name = (const char *)data;
        canvas_delete_slot_locked(name);
        bool ok = storage_art_delete(STORAGE_ART_PAINTING, name);
        if (ok)
        {
            ESP_LOGI(TAG, "deleted ");
        }
        break;
    }
    case SCENE_CANVAS_DRAW_BUF:
    {
        if (data == NULL)
            break;
        char *buf = (char *)data;
        canvas_draw_buf_locked(buf);
        break;
    }
    case SCENE_CANVAS_FILL_COLOR:
    {
        int color = (int)(data);
        canvas_fill_color_locked(color);
        break;
    }
    default:
        break;
    }

    display_mux_unlock();
}
void scene_init()
{
    canvas_obj = canvas_init();
    currency_obj = currency_init();
    clock_obj = clock_init();
    xTaskCreate(main_scene_task, "MainScene", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "initalized");
}
