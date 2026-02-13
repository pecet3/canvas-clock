#include "lvgl.h"
#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include "canvas.h"
#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64

static uint8_t canvas_buffer[(CANVAS_WIDTH * CANVAS_HEIGHT / 8) + 8];
static lv_obj_t *canvas = NULL;

static const char *TAG = "Canvas";

lv_obj_t *canvas_init(void)
{
    display_mux_lock();
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
    lv_obj_center(canvas);

    lv_canvas_set_palette(canvas, 0, lv_color_to_32(lv_color_hex(0x000000), LV_OPA_COVER)); // Indeks 0 = czarny
    lv_canvas_set_palette(canvas, 1, lv_color_to_32(lv_color_hex(0xFFFFFF), LV_OPA_COVER)); // Indeks 1 = biały

    lv_canvas_fill_bg(canvas, lv_color_hex(1), LV_OPA_COVER);

    display_mux_unlock();
    ESP_LOGI(TAG, "initialized");
    return canvas;
}
void canvas_draw_pixel(int32_t x, int32_t y, bool color_index)
{
    display_mux_lock();
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT)
    {
        lv_canvas_set_px(canvas, x, y, lv_color_hex(color_index), LV_OPA_COVER);
    }
    display_mux_unlock();
}

void canvas_draw_pixels(canvas_pixel_t *pixels, size_t count)
{
    if (pixels == NULL || count == 0)
        return;

    display_mux_lock();

    for (size_t i = 0; i < count; i++)
    {
        if (pixels[i].x >= 0 && pixels[i].x < CANVAS_WIDTH &&
            pixels[i].y >= 0 && pixels[i].y < CANVAS_HEIGHT)
        {
            lv_canvas_set_px(canvas, pixels[i].x, pixels[i].y,
                             lv_color_hex(pixels[i].color), LV_OPA_COVER);
        }
    }
    display_mux_unlock();
}

void canvas_draw_buf(char *ptr)
{
    if (ptr == NULL)
        return;
    int x, y, color, offset;
    display_mux_lock();
    while (sscanf(ptr, "%d %d %d%n",
                  &x,
                  &y,
                  &color,
                  &offset) == 3)
    {
        lv_canvas_set_px(canvas, x, y,
                         lv_color_hex(color), LV_OPA_COVER);
        ptr += offset;
    }
    display_mux_unlock();
}

void canvas_fill_color(uint32_t color)
{
    display_mux_lock();

    lv_canvas_fill_bg(canvas, lv_color_hex(color), LV_OPA_COVER);
    display_mux_unlock();
}
