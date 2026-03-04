#pragma once

bool buzzer_play_note_char(char note);
void buzzer_play_tone(uint32_t freq_hz, uint32_t duration_ms);
bool buzzer_play_note_string(const char *note_str);
void buzzer_init(void);
