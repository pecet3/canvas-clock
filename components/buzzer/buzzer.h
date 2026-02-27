#pragma once

bool buzzer_play_note_char(char note);
void buzzer_play_tone(uint32_t freq_hz, uint32_t duration_ms);
void buzzer_init(void);
