#pragma once

#include <stdint.h>

void init_audio_player(uint8_t *audio_data);

void play_audio_player(void);

void stop_audio_player(void);

extern uint8_t MAIN_THEME_START[];
extern uint8_t TITLE_SCREEN_START[];
