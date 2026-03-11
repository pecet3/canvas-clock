#include "lvgl.h"
#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include "canvas.h"
#include <stdint.h>
#include "storage.h"
#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64
#define CANVAS_BUF_SIZE (CANVAS_WIDTH * CANVAS_HEIGHT / 8) + 8
static uint8_t canvas_buffer[CANVAS_BUF_SIZE] __attribute__((aligned(4)));
static lv_obj_t *canvas = NULL;
static const char *TAG = "Canvas";

void canvas_get_drawing_buf(const char *dst, size_t size)
{
    if (size != CANVAS_BUF_SIZE)
    {
        ESP_LOGE(TAG, "Wrong buf size!");
    }
    display_mux_lock();
    memcpy(dst, canvas_buffer, CANVAS_BUF_SIZE);
    display_mux_unlock();
}

void canvas_set_drawing_buf(const char *src, size_t size)
{
    if (size != CANVAS_BUF_SIZE)
    {
        ESP_LOGE(TAG, "Wrong buf size!");
    }
    display_mux_lock();
    memcpy(canvas_buffer, src, CANVAS_BUF_SIZE);
    lv_obj_invalidate(canvas);
    display_mux_unlock();
}
/*

    Canvas drawing

*/

// lv_obj_invalidate();

void canvas_draw_pixel(int32_t x, int32_t y, bool color_index)
{
    display_mux_lock();
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT)
    {
        lv_canvas_set_px(canvas, x, y, lv_color_hex(color_index), LV_OPA_COVER);
    }
    display_mux_unlock();
}
void canvas_draw_pixel_locked(int32_t x, int32_t y, bool color_index)
{
    lv_canvas_set_px(canvas, x, y, lv_color_hex(color_index), LV_OPA_COVER);
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
void canvas_draw_buf_locked(char *ptr)
{
    if (ptr == NULL)
        return;
    int x, y, color, offset;
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
}
void canvas_fill_color_locked(uint32_t color)
{
    lv_canvas_fill_bg(canvas, lv_color_hex(color), LV_OPA_COVER);
}

/*

   STORAGE

*/

bool get_path(const char *name, char *buf, size_t buf_size)
{
    if (buf == NULL)
        return false;

    snprintf(buf, buf_size, "/spiffs/c/%s.bin", name);
    ESP_LOGI(TAG, "%s", buf);
    return true;
}

bool canvas_save_slot_locked(const char *name)
{
    char file_path[64];
    get_path(name, file_path, sizeof(file_path));
    FILE *file = fopen(file_path, "wb");
    if (file == NULL)
    {
        return false;
    }
    size_t n = fwrite(canvas_buffer, 1, CANVAS_BUF_SIZE, file);
    if (n != CANVAS_BUF_SIZE)
    {
        fclose(file);
        ESP_LOGE(TAG, "Failed to save file");
        return false;
    }
    fflush(file);
    fclose(file);
    return true;
}

bool canvas_load_slot_locked(const char *name)

{
    char file_path[64];
    get_path(name, file_path, sizeof(file_path));
    FILE *file = fopen(file_path, "rb");

    if (file == NULL)
    {
        ESP_LOGE(TAG, "No saved canvas found");
        return false;
    }
    fread(canvas_buffer, 1, CANVAS_BUF_SIZE, file);
    fclose(file);
    ESP_LOGI(TAG, "Canvas loaded from SPIFFS");
    return true;
}

bool canvas_delete_slot_locked(const char *name)
{
    char file_path[64];
    get_path(name, file_path, sizeof(file_path));
    if (remove(file_path) != 0)
    {

        ESP_LOGE(TAG, "Failed to delete canvas file");
        return false;
    }
    return true;
}

void canvas_set_drawing_locked()
{
    canvas_fill_color_locked(1);
    ESP_LOGI(TAG, "is drawing");
}

bool canvas_set_showing_locked()
{
    static int last_found_at_index = 0;

    storage_t *storage = storage_ptr();
    bool is_overlooped = false;
    bool found = false;

    storage_mux_lock();
    for (int i = last_found_at_index; i < 64; i++)
    {
        ESP_LOGI(TAG, "%s", storage->paintings[i].name);
        if (i == 63)
        {
            if (is_overlooped)
            {
                storage_mux_unlock();
                return false;
            }
            i = 0;
            is_overlooped = true;
        }
        if (storage->paintings[i].is_enabled)
        {
            ESP_LOGI(TAG, "this is: %d", storage->paintings[i].is_enabled);

            ESP_LOGI(TAG, "found at index: %d", last_found_at_index);
            last_found_at_index = i + 1;
            canvas_load_slot_locked(storage->paintings[i].name);
            found = true;
            break;
        }
    }
    ESP_LOGI(TAG, "found: %d", found);
    storage_mux_unlock();
    return found;
}

lv_obj_t *canvas_init(void)
{
    display_mux_lock();
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
    lv_canvas_set_palette(canvas, 1, lv_color_to_32(lv_color_hex(0x000000), LV_OPA_COVER));
    lv_canvas_set_palette(canvas, 0, lv_color_to_32(lv_color_hex(0xFFFFFF), LV_OPA_COVER));
    lv_obj_center(canvas);

    lv_canvas_fill_bg(canvas, lv_color_hex(0), LV_OPA_COVER);
    display_mux_unlock();
    ESP_LOG_BUFFER_HEX(TAG, canvas_buffer, CANVAS_BUF_SIZE);
    ESP_LOGI(TAG, "initialized");
    return canvas;
}