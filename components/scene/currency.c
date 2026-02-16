#include "currency.h"
#include "display.h"
#include "esp_log.h"
#include "data_fetcher.h"
#include <stdio.h>

static const char *TAG = "currency";

static lv_obj_t *currency_screen;
static lv_obj_t *usd_label;
static lv_obj_t *eur_label;
static lv_obj_t *gbp_label;
static lv_obj_t *czk_label;

static void create_row(lv_obj_t *parent, const char *name, lv_obj_t **value_label)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 5, 0);

    lv_obj_t *name_label = lv_label_create(row);
    lv_label_set_text(name_label, name);

    *value_label = lv_label_create(row);
    lv_label_set_text(*value_label, "...");
}

static void create_currency_ui(void)
{
    display_mux_lock();

    currency_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(currency_screen, lv_pct(100), lv_pct(100));
    lv_obj_center(currency_screen);

    lv_obj_set_flex_flow(currency_screen, LV_FLEX_FLOW_COLUMN);

    create_row(currency_screen, "USD:", &usd_label);
    create_row(currency_screen, "EUR:", &eur_label);
    create_row(currency_screen, "GBP:", &gbp_label);
    create_row(currency_screen, "CZK:", &czk_label);

    display_mux_unlock();
}

lv_obj_t *currency_init(void)
{
    create_currency_ui();
    ESP_LOGI(TAG, "currency screen created");
    return currency_screen;
}

void currency_update(fetch_data_t *data)
{
    if (!data)
        return;

    display_mux_lock();

    char buf[32];

    snprintf(buf, sizeof(buf), "%.4f", data->usd_mid);
    lv_label_set_text(usd_label, buf);

    snprintf(buf, sizeof(buf), "%.4f", data->eur_mid);
    lv_label_set_text(eur_label, buf);

    snprintf(buf, sizeof(buf), "%.4f", data->gbp_mid);
    lv_label_set_text(gbp_label, buf);

    snprintf(buf, sizeof(buf), "%.4f", data->czk_mid);
    lv_label_set_text(czk_label, buf);

    display_mux_unlock();
}
