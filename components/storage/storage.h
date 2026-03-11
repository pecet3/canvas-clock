#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// typedef struct
// {
//     int minute;
//     int hour;
//     int delay_after_notes_ms;
//     char painting_name[16];
//     char melody_name[16];
// } storage_alarm_t;

// typedef struct
// {
//     bool is_enabled;
//     int interval_minutes;
//     char melody[32];
// } storage_alarm_interval_t;

typedef struct
{
    char name[16];
    int len;
    char *rom;
} storage_chip8_rom_t;

typedef struct
{
    bool is_enabled;
    int created_at;
    char name[16];
} storage_art_t;

typedef struct
{
    storage_art_t paintings[64];
    storage_art_t melodies[64];
    storage_chip8_rom_t chip8_roms[64];
} storage_t;

typedef enum
{
    STORAGE_ART_PAINTING,
    STORAGE_ART_MELODY,
} storage_art_kind_t;

bool storage_art_set(storage_art_kind_t kind, const char *name);
bool storage_art_delete(storage_art_kind_t kind, const char *name);
storage_chip8_rom_t *storage_get_chip8_rom(const char *name);

void storage_mux_lock();
void storage_mux_unlock();
storage_t *storage_ptr();
void storage_init();