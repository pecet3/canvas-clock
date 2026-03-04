#include "lvgl.h"
#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include "canvas.h"
#include "nvs_flash.h"

#include <stdatomic.h>
#include <stdint.h>

#define NVS_NAMESPACE "storage"
#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 64
#define CANVAS_BUF_SIZE (CANVAS_WIDTH * CANVAS_HEIGHT / 8) + 8

// counts from 0
#define MAX_SLOTS 32 - 1

static uint8_t canvas_buffer[CANVAS_BUF_SIZE] __attribute__((aligned(4)));
static lv_obj_t *canvas = NULL;
static const char *TAG = "Canvas";

/*

    Canvas drawing

*/
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

/*

    NVS

*/

const char *canvas_get_nvs_slot_key(uint8_t num)
{
    if (num > MAX_SLOTS)
        return NULL;
    char nvs_key[16];
    snprintf(nvs_key, sizeof(nvs_key), "c_slot%d", num);
    return strdup(nvs_key);
}

bool canvas_save_slot_locked(const char *nvs_key)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        return false;
    }

    size_t buf_size = CANVAS_BUF_SIZE;
    char buf[buf_size];
    memcpy(buf, canvas_buffer, buf_size);
    err = nvs_set_blob(nvs_handle, nvs_key, &buf, buf_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save canvas buffer to NVS: %s", esp_err_to_name(err));
        goto cleanup_err;
    }
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit changes to NVS: %s", esp_err_to_name(err));
        goto cleanup_err;
    }

    ESP_LOGI(TAG, "Canvas buffer saved to NVS with key: %s", nvs_key);
    nvs_close(nvs_handle);
    return true;

cleanup_err:
    nvs_close(nvs_handle);
    return false;
}

bool canvas_load_slot_locked(const char *nvs_key)
{
    nvs_handle_t nvs_handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle");
        return false;
    }
    size_t buf_size = CANVAS_BUF_SIZE;
    char buf[buf_size];
    esp_err_t err = nvs_get_blob(nvs_handle, nvs_key, &buf, &buf_size);
    nvs_close(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to load canvas buffer from NVS: %s KEY: %s", esp_err_to_name(err), nvs_key);
        return false;
    }

    memcpy(canvas_buffer, buf, buf_size);
    ESP_LOGI(TAG, "Canvas buffer loaded from NVS with key: %s", nvs_key);
    return true;
}

bool canvas_delete_slot_locked(const char *nvs_key)
{
    nvs_handle_t nvs_handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle");
        return false;
    }
    esp_err_t err = nvs_erase_key(nvs_handle, nvs_key);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete canvas buffer from NVS: %s", esp_err_to_name(err));
        goto cleanup_err;
    }
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit changes to NVS: %s", esp_err_to_name(err));
        goto cleanup_err;
    }

    ESP_LOGI(TAG, "Canvas buffer deleted from NVS with key: %s", nvs_key);
    nvs_close(nvs_handle);
    return true;

cleanup_err:
    nvs_close(nvs_handle);
    return false;
}

void canvas_load_slot(uint8_t slot_num)
{
    display_mux_lock();
    const char *nvs_key = canvas_get_nvs_slot_key(slot_num);
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
    const char *nvs_key = canvas_get_nvs_slot_key(slot_num);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Invalid slot number: %d", slot_num);
        display_mux_unlock();
        return;
    }
    canvas_save_slot_locked(nvs_key);
    display_mux_unlock();
}
void canvas_delete_slot(uint8_t slot_num)
{
    display_mux_lock();
    const char *nvs_key = canvas_get_nvs_slot_key(slot_num);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Invalid slot number: %d", slot_num);
        display_mux_unlock();
        return;
    }
    canvas_delete_slot_locked(nvs_key);
    display_mux_unlock();
}

void canvas_set_drawing_locked()
{
    canvas_fill_color_locked(1);
    ESP_LOGI(TAG, "is drawing");
}

static _Atomic uint8_t current_slot = 0;
void canvas_set_current_slot(uint8_t new_value)
{
    atomic_store_explicit(&current_slot, new_value, memory_order_relaxed);
}

uint8_t canvas_get_current_slot()
{
    return atomic_load_explicit(&current_slot, memory_order_relaxed);
}

bool canvas_set_showing_locked()
{
    const char *nvs_key = canvas_get_nvs_slot_key(current_slot);
    if (nvs_key == NULL)
    {
        ESP_LOGE(TAG, "Invalid slot number: %d", current_slot);
        return false;
    }
    if (canvas_load_slot_locked(nvs_key))
    {
        canvas_set_current_slot(canvas_get_current_slot() + 1);
        return true;
    }

    ESP_LOGE(TAG, "Failed to load canvas slot %d", current_slot);

    // stop recursion if we looped through all slots and found none
    if (canvas_get_current_slot() == MAX_SLOTS)
    {
        ESP_LOGI(TAG, "No more slots to show, resetting to slot 0 and stopping recursion");
        current_slot = 0;
        return canvas_set_showing_locked();
    }

    canvas_set_current_slot(canvas_get_current_slot() + 1);
    canvas_set_showing_locked();
    return true;
}

lv_obj_t *canvas_init(void)
{
    display_mux_lock();
    canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, canvas_buffer, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_I1);
    lv_canvas_set_palette(canvas, 0, lv_color_to_32(lv_color_hex(0x000000), LV_OPA_COVER));
    lv_canvas_set_palette(canvas, 1, lv_color_to_32(lv_color_hex(0xFFFFFF), LV_OPA_COVER));
    lv_obj_center(canvas);

    lv_canvas_fill_bg(canvas, lv_color_hex(1), LV_OPA_COVER);
    display_mux_unlock();
    ESP_LOGI(TAG, "initialized");
    return canvas;
}