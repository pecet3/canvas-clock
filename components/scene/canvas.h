

#include "lvgl.h"
#pragma once

typedef struct canvas_pixel
{
    int x;
    int y;
    int color;
} canvas_pixel_t;

lv_obj_t *
canvas_init(void);
void canvas_draw_pixel(int32_t x, int32_t y, bool color_index);
void canvas_draw_pixels(canvas_pixel_t *pixels, size_t count);
// colors: 0 or 1
void canvas_fill_color(uint32_t color);
void canvas_draw_buf(char *ptr);
lv_obj_t *canvas_get_lvgl_obj(void);

void canvas_save_slot(uint8_t slot_num);
void canvas_load_slot(uint8_t slot_num);
