#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    int minute;
    int hour;
    int delay_after_notes_ms;
    char painting_name[16];
    char melody_name[16];
} config_alarm_t;

typedef struct
{
    bool is_enabled;
    int interval_minutes;
    char melody[32];
} config_alarm_interval_t;

typedef struct
{
    bool is_enabled;
    __time_t created_at;
    char name[16];
} config_art_t;

typedef struct
{
    config_alarm_t alarms[32];
    config_alarm_interval_t alarm_interval;
    config_art_t paintings[64];
    config_art_t melodies[64];
} config_t;