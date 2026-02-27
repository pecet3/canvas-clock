#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <string.h>

#define BUZZER_PIN 4

#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494

// task tag
static const char *TAG = "BUZZER";

/** FUNCTIONS **/

void buzzer_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = BUZZER_PIN,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&ledc_channel);
}

void buzzer_play_tone(uint32_t freq_hz, uint32_t duration_ms)
{

    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq_hz);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4096);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    vTaskDelay(duration_ms / portTICK_PERIOD_MS);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
bool buzzer_play_note_char(char note)
{
    switch (note)
    {
    case 'C':
        buzzer_play_tone(NOTE_C4, 200);
        break;
    case 'D':
        buzzer_play_tone(NOTE_D4, 200);
        break;
    case 'E':
        buzzer_play_tone(NOTE_E4, 200);
        break;
    case 'F':
        buzzer_play_tone(NOTE_F4, 200);
        break;
    case 'G':
        buzzer_play_tone(NOTE_G4, 200);
        break;
    case 'A':
        buzzer_play_tone(NOTE_A4, 200);
        break;
    case 'B':
        buzzer_play_tone(NOTE_B4, 200);
        break;

    // Halftones
    case 'c':
        buzzer_play_tone(NOTE_CS4, 200);
        break; // C#
    case 'd':
        buzzer_play_tone(NOTE_DS4, 200);
        break; // D#
    case 'f':
        buzzer_play_tone(NOTE_FS4, 200);
        break; // F#
    case 'g':
        buzzer_play_tone(NOTE_GS4, 200);
        break; // G#
    case 'a':
        buzzer_play_tone(NOTE_AS4, 200);
        break; // A#
    case ' ':
        vTaskDelay(200 / portTICK_PERIOD_MS);
        break; //
    default:
        return false;
        break;
    }
    return true;
}