#include "lvgl.h"
#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include "canvas.h"
#include "nvs_flash.h"
#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64
#define CANVAS_BUF_SIZE (CANVAS_WIDTH * CANVAS_HEIGHT / 8) + 8
static uint8_t drawing_buf[CANVAS_BUF_SIZE] __attribute__((aligned(4)));
static uint8_t showing_buf[CANVAS_BUF_SIZE] __attribute__((aligned(4)));
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
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
        return;
    ;
    size_t buf_size = CANVAS_BUF_SIZE;
    nvs_set_blob(nvs_handle, nvs_key, drawing_buf, buf_size);
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
bool canvas_load_slot_locked(const char *nvs_key)
{
    nvs_handle_t nvs_handle;
    if (nvs_open("storage", NVS_READONLY, &nvs_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle");
        return false;
    }
    size_t buf_size = CANVAS_BUF_SIZE;
    uint8_t *buf = showing_buf;
    esp_err_t err = nvs_get_blob(nvs_handle, nvs_key, buf, &buf_size);
    nvs_close(nvs_handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to load canvas buffer from NVS: %s", esp_err_to_name(err));
        return false;
    }
    else
    {
        lv_canvas_set_buffer(canvas, showing_buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
        ESP_LOGI(TAG, "Canvas buffer loaded from NVS with key: %s", nvs_key);
        return true;
    }
}
// nfs
const char *canvas_get_nvs_key(uint8_t num)
{
    if (num > 15)
        return NULL;
    char nvs_key[16];
    snprintf(nvs_key, sizeof(nvs_key), "canvas%d", num);
    return strdup(nvs_key);
}
void canvas_load_slot(uint8_t slot_num)
{
    display_mux_lock();

    const char *nvs_key = canvas_get_nvs_key(slot_num);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Invalid slot number: %d", slot_num);
        display_mux_unlock();
        return;
    }
    canvas_load_slot_locked(nvs_key);
    display_mux_unlock();
}

void canvas_save_slot(uint8_t slot_num)
{
    display_mux_lock();
    const char *nvs_key = canvas_get_nvs_key(slot_num);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Invalid slot number: %d", slot_num);
        display_mux_unlock();
        return;
    }
    canvas_save_slot_locked(nvs_key);
    display_mux_unlock();
}

//
void canvas_set_drawing_locked()
{
    lv_canvas_set_buffer(canvas, drawing_buf,
                         CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
}

static uint8_t current_slot = 0;
void canvas_set_showing_locked()
{
    const char *nvs_key = canvas_get_nvs_key(current_slot);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Invalid slot number: %d", current_slot);
        return;
    }
    if (!canvas_load_slot_locked(nvs_key))
    {
        ESP_LOGE(TAG, "Failed to load canvas slot %d", current_slot);
        current_slot = 0;
        canvas_set_showing_locked();
    }
    else
    {
        current_slot++;
        lv_canvas_set_buffer(canvas, showing_buf,
                             CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
    }
}

lv_obj_t *canvas_init(void)
{
    display_mux_lock();
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, drawing_buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
    lv_canvas_set_palette(canvas, 0, lv_color_to_32(lv_color_hex(0x000000), LV_OPA_COVER));
    lv_canvas_set_palette(canvas, 1, lv_color_to_32(lv_color_hex(0xFFFFFF), LV_OPA_COVER));
    lv_obj_center(canvas);

    lv_canvas_fill_bg(canvas, lv_color_hex(1), LV_OPA_COVER);
    display_mux_unlock();
    ESP_LOGI(TAG, "initialized");
    return canvas;
}