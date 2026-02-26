#include "lvgl.h"
#include "display.h"
#include "esp_log.h"
#include <time.h>
#include <stdio.h>
#include "font.h"
static const char *TAG = "Clock";
static lv_obj_t *clock_screen;
static lv_obj_t *clock_label;
static lv_obj_t *date_label;
// static void create_clock_ui(void)
// {
//     display_mux_lock();

//     clock_screen = lv_obj_create(lv_scr_act());
//     lv_obj_set_size(clock_screen, lv_pct(100), lv_pct(100));
//     lv_obj_center(clock_screen);

//     lv_obj_set_style_bg_color(clock_screen, lv_color_hex(0x000000), 0);
//     lv_obj_set_style_border_width(clock_screen, 0, 0);
//     lv_obj_set_style_radius(clock_screen, 0, 0);

//     clock_label = lv_label_create(clock_screen);
//     lv_obj_set_style_text_font(clock_label, get_font_changa24i_num(), 0);
//     lv_obj_set_style_text_color(clock_label, lv_color_hex(0xFFFFFF), 0);
//     lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, -10);
//     lv_label_set_text(clock_label, "00:00:00");

//     lv_obj_set_style_text_font(date_label, get_font_terminus12(), 0);
//     lv_obj_set_style_text_color(date_label, lv_color_hex(0xFFFFFF), 0);

//     lv_obj_align_to(date_label, clock_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
//     lv_label_set_text(date_label, "2026-02-20");

//     display_mux_unlock();
// }
static void create_clock_ui(void)
{
    display_mux_lock();

    clock_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(clock_screen, lv_pct(100), lv_pct(100));
    lv_obj_center(clock_screen);
    lv_obj_set_style_pad_all(clock_screen, 0, 0);
    lv_obj_set_style_border_width(clock_screen, 0, 0);

    clock_label = lv_label_create(clock_screen);
    lv_obj_set_style_text_font(clock_label, get_font_workbench_30num(), 0);
    lv_obj_align(clock_label, LV_ALIGN_CENTER, 0, -10);

    date_label = lv_label_create(clock_screen);
    lv_obj_set_style_text_font(date_label, get_font_terminus12(), 0);

    lv_obj_align_to(date_label, clock_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_label_set_text(date_label, "2024-01-01");

    display_mux_unlock();
}
static void clock_timer_cb(lv_timer_t *timer)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char buf_clock[9];
    snprintf(buf_clock, sizeof(buf_clock), "%02d:%02d:%02d",
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    lv_label_set_text(clock_label, buf_clock);

    char buf_date[16];
    strftime(buf_date, sizeof(buf_date), "%Y-%m-%d", &timeinfo);
    lv_label_set_text(date_label, buf_date);
}

lv_obj_t *clock_init(void)
{
    create_clock_ui();

    lv_timer_t *timer = lv_timer_create(clock_timer_cb, 1000, NULL);
    ESP_LOGI(TAG, "initialized");

    return clock_screen;
}
