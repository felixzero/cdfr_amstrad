.module interrupt

.globl interrupt_service_routine
.globl vsync_counter
.globl PLY_AKM_PLAY
.globl audio_play

VSYNC_IN = 0xF5

.area _CODE

interrupt_service_routine:
    ; Backup registers
    push af
    push bc
    push de
    push hl
    push ix
    push iy

    ; Check if we are in VSync
    ld b, #VSYNC_IN
    in a, (c)
    rra
    jr NC, reti$

    ; Increase counter
    ld hl, #vsync_counter
    inc (hl)

    ; Play audio
    ld a, (#audio_play)
    or a
    jr Z, skip_audio$
    call PLY_AKM_PLAY
skip_audio$:

    ; Restore registers and return
reti$:
    pop iy
    pop ix
    pop hl
    pop de
    pop bc
    pop af
    ei
    reti

vsync_counter:
    .db 0
