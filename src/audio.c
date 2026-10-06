#include "audio.h"

void init_audio_player(uint8_t *audio_data)
{
    (void*)audio_data;
    __asm
    push ix
    xor a
    call PLY_AKM_INIT
    pop ix
    __endasm; 
}

void play_audio_player(void)
{
    __asm
    push ix
    call PLY_AKM_PLAY
    pop ix
    __endasm;
}

void stop_audio_player(void)
{
    __asm
    push ix
    call PLY_AKM_STOP
    pop ix
    __endasm;
}
