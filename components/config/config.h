#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    int minute;
    int hour;
    int canvas_slot;
    int delay_after_notes_ms;
    char *buzzer_notes;
} config_alarm_t;

typedef struct
{
    bool is_enabled;
    int interval_minutes;
    char *buzzer_interval_notes;
} config_alarm_interval_t;

typedef struct
{
    config_alarm_t *alarms_array;
    size_t alarms_array_size;
    config_alarm_interval_t alarm_interval;
    int *slots_order_array;
    int slots_order_size;
} config_t;