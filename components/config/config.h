#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    int minute;
    int hour;
    int delay_after_notes_ms;
    uint8_t canvas_slot;
    uint8_t buzzer_slot;
} config_alarm_t;

typedef struct
{
    bool is_enabled;
    int interval_minutes;
    uint8_t buzzer_slot;
} config_alarm_interval_t;

typedef struct
{
    config_alarm_t alarms[32];
    config_alarm_interval_t alarm_interval;
    uint8_t canvas_slot_order[32];
} config_t;