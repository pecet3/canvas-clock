

#include "lvgl.h"
#pragma once

typedef struct canvas_pixel
{
    int x;
    int y;
    int color;
} canvas_pixel_t;

lv_obj_t *canvas_init(void);
void canvas_draw_pixel(int32_t x, int32_t y, bool color_index);
void canvas_draw_pixels(canvas_pixel_t *pixels, size_t count);
// colors: 0 or 1
void canvas_fill_color(uint32_t color);
void canvas_draw_buf(char *ptr);
void canvas_draw_buf_locked(char *ptr);

void canvas_save_slot(uint8_t slot_num);
void canvas_load_slot(uint8_t slot_num);
void canvas_delete_slot(uint8_t slot_num);
const char *canvas_get_nvs_slot_key(uint8_t num);
bool canvas_save_slot_locked(const char *nvs_key);
bool canvas_load_slot_locked(const char *nvs_key);
bool canvas_delete_slot_locked(const char *nvs_key);

void canvas_set_current_slot(uint8_t new_value);
uint8_t canvas_get_current_slot();
