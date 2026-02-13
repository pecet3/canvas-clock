#include "scene.h"
#include "lvgl.h"
#include "canvas.h"
#include "clock.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
extern lv_obj_t *clock_get_lvgl_obj(void);
extern lv_obj_t *canvas_get_lvgl_obj(void);
static scene_t current_scene = SCENE_CLOCK;
void scene_set(scene_t scene)
{
    display_mux_lock();
    if (current_scene == SCENE_CLOCK)
    {
        lv_obj_clear_flag(clock_get_lvgl_obj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(canvas_get_lvgl_obj(), LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(clock_get_lvgl_obj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(canvas_get_lvgl_obj(), LV_OBJ_FLAG_HIDDEN);
    }
    display_mux_unlock();
    current_scene = scene;
}

void scene_init()
{
    canvas_start();
    clock_start();
}
