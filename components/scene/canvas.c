#include "lvgl.h"
#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include "canvas.h"
#include "nvs_flash.h"
#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64
#define CANVAS_BUF_SIZE (CANVAS_WIDTH * CANVAS_HEIGHT / 8) + 8
static uint8_t canvas_buffer[CANVAS_BUF_SIZE];
static lv_obj_t *canvas = NULL;

static const char *TAG = "Canvas";

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

void canvas_fill_color_locked(uint32_t color)
{
    lv_canvas_fill_bg(canvas, lv_color_hex(color), LV_OPA_COVER);
}

void canvas_fill_color(uint32_t color)
{
    display_mux_lock();
    canvas_fill_color_locked(color);
    display_mux_unlock();
}

void canvas_save_slot_locked(const char *nvs_key)
{
    nvs_key = "canvas1";

    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
        return;

    const uint8_t *buf = canvas_buffer;
    size_t buf_size = CANVAS_BUF_SIZE;
    nvs_set_blob(nvs_handle, nvs_key, buf, buf_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save canvas buffer to NVS: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Canvas buffer saved to NVS with key: %s", nvs_key);
    }

    err = nvs_commit(nvs_handle);

    nvs_close(nvs_handle);
}
void canvas_load_slot_locked(const char *nvs_key)
{
    nvs_key = "canvas1";
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READONLY, &nvs_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle");
        return;
    }

    size_t buf_size = CANVAS_BUF_SIZE;
    uint8_t buf[buf_size];
    esp_err_t err = nvs_get_blob(nvs_handle, nvs_key, buf, &buf_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to load canvas buffer from NVS: %s", esp_err_to_name(err));
    }
    else
    {
        lv_canvas_set_buffer(canvas, buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
        ESP_LOGI(TAG, "Canvas buffer loaded from NVS with key: %s", nvs_key);
    }
}

const char *canvas_get_nvs_key(int num)
{
    switch (num)
    {
    case 1:
        return "canvas1";
    case 2:
        return "canvas2";
    case 3:
        return "canvas3";
    case 4:
        return "canvas4";
    default:
        return NULL;
    }
}
void canvas_load_slot(int slot_num)
{
    display_mux_lock();
    const char *nvs_key = canvas_get_nvs_key(slot_num);
    canvas_load_slot_locked(nvs_key);
    display_mux_unlock();
}

void canvas_save_slot(int slot_num)
{
    display_mux_lock();
    const char *nvs_key = canvas_get_nvs_key(slot_num);

    canvas_save_slot_locked(nvs_key);
    display_mux_unlock();
}

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