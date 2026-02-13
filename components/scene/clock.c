#include "lvgl.h"
#include "display.h"
#include "esp_log.h"
#include <time.h>
#include <stdio.h>
static const char *TAG = "Clock";
static lv_obj_t *clock_label;

static void create_clock_ui(void)
{
    display_mux_lock();
    clock_label = lv_label_create(lv_scr_act());
    lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(clock_label, lv_font_default(), 0);
    lv_label_set_text(clock_label, "Loading...");
    display_mux_unlock();
}
static void clock_timer_cb(lv_timer_t *timer)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    lv_label_set_text(clock_label, buf);
}

lv_obj_t *clock_init(void)
{
    create_clock_ui();
    lv_timer_t *timer = lv_timer_create(clock_timer_cb, 1000, NULL);
    ESP_LOGI(TAG, "initialized");

    return clock_label;
}
