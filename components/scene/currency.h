#pragma once
#include "data_fetcher.h"
#include "lvgl.h"
void currency_update(fetch_data_t *data);
lv_obj_t *currency_init(void);