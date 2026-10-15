#pragma once

#include <stdint.h>

enum sound_effect {
    SOUND_EFFECT_SILENCE,
    SOUND_EFFECT_MINE,
    SOUND_EFFECT_BUILD,
    SOUND_EFFECT_GRAIL,
    SOUND_EFFECT_BOINK
};

// Configure the audio player for a specific song
void init_audio_player(uint8_t *audio_data);

// Stop sound playback
void stop_audio_player(void);

// Trigger the playing of a sound effect
void play_sound_effect(uint8_t effect);


extern uint8_t SOUND_EFFECTS_SOUNDEFFECTS[];
extern uint8_t MAIN_MUSIC_START[];
extern uint8_t TITLE_SCREEN_START[];

