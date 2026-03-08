

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
// colors: 0 or 1);
void canvas_draw_buf_locked(char *ptr);

bool canvas_get_painting_path(uint8_t num, char *buf, size_t buf_size);
bool canvas_get_painting_path_name(const char *name, char *buf, size_t buf_size);
bool canvas_save_slot_locked(const char *file_name);
bool canvas_load_slot_locked(const char *file_name);
bool canvas_delete_slot_locked(const char *file_name);
