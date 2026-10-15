.module audio

; Exports
.globl _init_audio_player
.globl _stop_audio_player
.globl _play_sound_effect
.globl audio_play

; Imports
.globl PLY_AKM_INIT
.globl PLY_AKM_INITSOUNDEFFECTS
.globl PLY_AKM_STOP
.globl PLY_AKM_PLAYSOUNDEFFECT
.globl _SOUND_EFFECTS_SOUNDEFFECTS

.area _CODE

_init_audio_player:
    push ix

    xor a
    call PLY_AKM_INIT

    ld hl, #_SOUND_EFFECTS_SOUNDEFFECTS
    call PLY_AKM_INITSOUNDEFFECTS

    ld a, #1
    ld (#audio_play), a

    pop ix
    ret


_stop_audio_player:
    push ix
    call PLY_AKM_STOP

    xor a
    ld (#audio_play), a

    pop ix
    ret


_play_sound_effect:
    push ix

    ld c, #0
    ld b, #0
    call PLY_AKM_PLAYSOUNDEFFECT

    pop ix
    ret


audio_play:
    .db 0
