#include "scene.h"
#include "canvas.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "buzzer.h"
#include <time.h>
#include <stdio.h>

static const char *TAG = "Alarm";
static void alarm_task(void *arg)
{
    ESP_LOGI(TAG, "Alarm task started");

    int last_hour_played = -1;
    bool barka_played_today = false;
    int last_barka_day = -1;

    while (1)
    {
        vTaskDelay((800) / portTICK_PERIOD_MS);

        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_sec == 0 && timeinfo.tm_min % 10 == 0)
        {

            buzzer_play_note_string("CC E");
        }
        // Co godzinę o xx:00:00 – Panie Janie
        if (timeinfo.tm_min == 0 &&
            timeinfo.tm_sec == 0 &&
            timeinfo.tm_hour != last_hour_played)
        {
            last_hour_played = timeinfo.tm_hour;

            ESP_LOGI(TAG, "Hour signal at %02d:00:00", timeinfo.tm_hour);

            buzzer_play_note_string("C D E C  C D E C  E F G  E F G  G A G F E C  G A G F E C  C G C  C G C");
        }

        // O 21:37 – Barka (raz dziennie)
        if (timeinfo.tm_hour == 21 &&
            timeinfo.tm_min == 37 &&
            timeinfo.tm_sec == 0 &&
            (timeinfo.tm_mday != last_barka_day))
        {
            last_barka_day = timeinfo.tm_mday;

            ESP_LOGI(TAG, "21:37 - Barka time");

            uint8_t prev = canvas_get_current_slot();
            canvas_set_current_slot(1);
            scene_set(SCENE_CANVAS_SHOW, NULL);

            // Uproszczona melodia „Barka” – dostosuj do swojego buzzera
            buzzer_play_note_string("G A B C B A G  A B C D C B A  G A B C B A G");

            vTaskDelay(8000 / portTICK_PERIOD_MS);

            canvas_set_current_slot(prev);
            scene_set(SCENE_MAIN, NULL);
        }
    }
}

void alarm_init()
{
    xTaskCreate(alarm_task, "AlarmTask", 4096, NULL, 10, NULL);
}