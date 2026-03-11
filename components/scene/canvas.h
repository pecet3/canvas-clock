

#include "lvgl.h"
#pragma once

#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64
#define CANVAS_BUF_SIZE (CANVAS_WIDTH * CANVAS_HEIGHT / 8) + 8

typedef struct canvas_pixel
{
    int x;
    int y;
    int color;
} canvas_pixel_t;

lv_obj_t *canvas_init(void);

void canvas_get_drawing_buf(const char *dst, size_t size);
void canvas_set_drawing_buf(const char *src, size_t size);

void canvas_draw_pixel(int32_t x, int32_t y, bool color_index);
void canvas_draw_pixel_locked(int32_t x, int32_t y, bool color_index);
void canvas_draw_pixels(canvas_pixel_t *pixels, size_t count);
// colors: 0 or 1);
void canvas_draw_buf_locked(char *ptr);

bool canvas_get_painting_path(uint8_t num, char *buf, size_t buf_size);
bool canvas_get_painting_path_name(const char *name, char *buf, size_t buf_size);
bool canvas_save_slot_locked(const char *file_name);
bool canvas_load_slot_locked(const char *file_name);
bool canvas_delete_slot_locked(const char *file_name);
