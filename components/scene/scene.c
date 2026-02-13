#include "scene.h"
#include "lvgl.h"
#include "canvas.h"
#include "clock.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "Scene";
static lv_obj_t *canvas_obj;
static lv_obj_t *clock_obj;
static scene_t current_scene = SCENE_CLOCK;

void scene_set(scene_t scene)
{
    ESP_LOGI(TAG, "currenct scene: %d set to: %d", current_scene, scene);
    display_mux_lock();
    if (current_scene == SCENE_CLOCK)
    {
        lv_obj_clear_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(canvas_obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(clock_obj, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(canvas_obj, LV_OBJ_FLAG_HIDDEN);
    }
    display_mux_unlock();
    current_scene = scene;
}

void scene_init()
{
    canvas_obj = canvas_init();
    clock_obj = clock_init();
    ESP_LOGI(TAG, "initalized");
}
