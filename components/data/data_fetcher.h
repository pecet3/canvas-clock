#pragma once
#include <stdio.h>

typedef struct fetch_data
{
    double usd_mid;
    double eur_mid;
    double gbp_mid;
    double czk_mid;
} fetch_data_t;

bool get_fetch_data(fetch_data_t *data);
void data_fetcher_init(void);