#include "currency.h"
#include "display.h"
#include "esp_log.h"
#include "data_fetcher.h"
#include <stdio.h>
#include "font/font.h"
static const char *TAG = "currency";

static lv_obj_t *currency_screen;
static lv_obj_t *currency_label;
static char currency_text[128];

static void create_currency_objects_locked(void)
{
    currency_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(currency_screen, lv_pct(100), lv_pct(100));
    lv_obj_center(currency_screen);
    currency_label = lv_label_create(currency_screen);
    lv_obj_align(currency_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(currency_label, "loading\n");
    lv_obj_set_style_text_font(currency_label, font_terminus12(), 0);
}

void currency_update(fetch_data_t *data)
{
    if (!data)
        return;

    display_mux_lock();
    snprintf(currency_text, sizeof(currency_text),
             "1 USD = %.4f PLN\n1 EUR = %.4f PLN\n1 GBP = %.4f PLN\n1 CHF = %.4f PLN\n",
             data->usd_mid, data->eur_mid, data->gbp_mid, data->chf_mid);

    lv_label_set_text(currency_label, currency_text);
    display_mux_unlock();
}

lv_obj_t *currency_init(void)
{
    display_mux_lock();
    create_currency_objects_locked();
    display_mux_unlock();
    fetch_data_t empty_data = {0};

    currency_update(&empty_data);

    ESP_LOGI(TAG, "currency screen created");
    return currency_screen;
}
