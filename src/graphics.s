.globl _wait_for_vsync
.globl _swap_buffers
.globl _set_double_buffering
.globl _set_palette
.globl _blit_sprite_xor
.globl _blit_sprite_swap
.globl _get_current_buffer
.globl interrupt_service_routine

.area _CODE

RECT_X = 0
RECT_Y = 1
RECT_W = 2
RECT_H = 3

CRTC_REG_SCROLL = 0xBC0C
VSYNC_IN = 0xF5

BUFFER_C000 = 0x30
BUFFER_4000 = 0x10
BUFFER_SWAP_XOR_MASK = 0x20

LINE_JUMP_OFFSET = 0x0800
BLOCK_JUMP_OFFSET = (-0xF800 + 0xC050)


; Block execution until the beginning of VSync
; Args: -
; Ret: -
; Modifies: AF, BC
_wait_for_vsync:
wait$:
    halt
    ld hl, #vsync_counter
    cp a, (hl)
    jr NC, wait$
    ld (hl), #0
    ret


; Swap the currently displayed buffer with the work buffer
; Args: -
; Ret: -
; Modifies: AF, BC
_swap_buffers:
    ; Set screen high address
    ld bc, #CRTC_REG_SCROLL
    out (c), c
    ld a, (#current_buffer)
    xor a, #BUFFER_SWAP_XOR_MASK
    ld (#current_buffer), a
    inc b
    out (c), a

    ret

; Configure the display into double buffering or single buffering
; Args: enabled (in A)
; Ret: -
; Modified: -
_set_double_buffering:
    or a
    jr NZ, double$

    ; Both work and display buffers are in C000
    ld bc, #CRTC_REG_SCROLL
    out (c), c
    ld a, #BUFFER_4000
    ld (#current_buffer), a
    xor a, #BUFFER_SWAP_XOR_MASK
    inc b
    out (c), a
    ret

double$:
    ; swap_buffers restores the default double-buffering behavior
    jp _swap_buffers



; Pick an indexed color as a palette element
; Args: A = index, L = color
; Ret: -
_set_palette:
    ld b, #0x7F
    ld c, a
    out (c), c

    ld c, l
    out (c), c

    ret


; Blit a sprite onto the work buffer, using a simple XOR strategy
; Args: HL = address to sprite data, DE = rect structure with the destination position
; Ret: -
; Modifies: AF, BC, DE, HL, IY
_blit_sprite_xor:
    ; IY := rect
    push de
    pop iy
    push hl
    pop de

    ; == Calculate screen destination address in HL ==
    ; (HL = 80 * (y / 8) + 0x0800 * (y % 8) + (0xC000 or 0x4000) + x)
    ; A = 0x08 * (y % 8)
    ld a, RECT_Y(iy)
    and #0x07
    sla a
    sla a
    sla a

    ; B = A + 0xC0 or 0x40
    add #0x40
    ld b, a
    ld a, (#current_buffer)
    bit 5, a
    jr NZ, skip$2
    set 7, b
skip$2:

    ; C = x
    ld a, RECT_X(iy)
    srl a
    ld c, a

    ; HL = 80 * (y / 8)
    ld a, RECT_Y(iy)
    srl a
    srl a
    srl a ; A = y / 8
    ld l, a
    sla a
    sla a
    add l ; A = 5 * (y / 8)
    ld l, a
    ld h, #0 ; HL = 5 * (y / 8)
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl ; HL = 80 * (y / 8)

    ; HL = 80 * (y / 8) + 0x0800 * (y % 8) + (0xC000 or 0x4000) + x
    add hl, bc

    ; FOR C = rect.h to 1 (loop over columns)
    ld c, RECT_H(iy)

loop_lines$2:
    ; Save beginning of line
    push hl
    ld b, RECT_W(iy)
    srl b

loop_columns$2:
    ; HL = screen, DE = sprite; perform swap
    ld a, (de)
    xor a, (hl)
    ld (hl), a
    inc hl
    inc de
    djnz loop_columns$2

    ; Calculate next line
    ; First restore beginning of line
    pop hl
    ; Next line = line + LINE_JUMP_OFFSET
    ld a, h
    add #(LINE_JUMP_OFFSET >> 8)
    ld h, a
    ; If bit 6 is 1, still inside the screen
    bit 6, h
    jr NZ, normal_line$2
    push de
    ; Jump into next block
    ld de, #(BLOCK_JUMP_OFFSET - LINE_JUMP_OFFSET)
    add hl, de
    pop de
normal_line$2:

    ; ENDFOR (loop over columns)
    dec c
    jr NZ, loop_lines$2

    ret


; Blit a sprite onto the work buffer, exchanging the content of the assets with a backup of the screen
; Args: HL = address to sprite data, DE = rect structure with the destination position
; Ret: -
; Modifies: AF, BC, DE, HL, IY
_blit_sprite_swap:
    ; IY := rect
    push de
    pop iy
    push hl
    pop de

    ; == Calculate screen destination address in HL ==
    ; (HL = 80 * (y / 8) + 0x0800 * (y % 8) + (0xC000 or 0x4000) + x)
    ; A = 0x08 * (y % 8)
    ld a, RECT_Y(iy)
    and #0x07
    sla a
    sla a
    sla a

    ; B = A + 0xC0 or 0x40
    add #0x40
    ld b, a
    ld a, (#current_buffer)
    bit 5, a
    jr NZ, skip$
    set 7, b
skip$:

    ; C = x
    ld a, RECT_X(iy)
    srl a
    ld c, a

    ; HL = 80 * (y / 8)
    ld a, RECT_Y(iy)
    srl a
    srl a
    srl a ; A = y / 8
    ld l, a
    sla a
    sla a
    add l ; A = 5 * (y / 8)
    ld l, a
    ld h, #0 ; HL = 5 * (y / 8)
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl ; HL = 80 * (y / 8)

    ; HL = 80 * (y / 8) + 0x0800 * (y % 8) + (0xC000 or 0x4000) + x
    add hl, bc

    ; FOR C = rect.h to 1 (loop over columns)
    ld c, RECT_H(iy)

loop_lines$1:
    ; Save beginning of line
    push hl
    ld b, RECT_W(iy)
    srl b

loop_columns$1:
    ; HL = sprite, DE = screen; perform swap
    ld a, (de)
    or a
    jr Z, skip_trans$
    push af
    ld a, (hl)
    ld (de), a
    pop af
    ld (hl), a
skip_trans$:
    inc hl
    inc de
    djnz loop_columns$1

    ; Calculate next line
    ; First restore beginning of line
    pop hl
    ; Next line = line + LINE_JUMP_OFFSET
    ld a, h
    add #(LINE_JUMP_OFFSET >> 8)
    ld h, a
    ; If bit 6 is 1, still inside the screen
    bit 6, h
    jr NZ, normal_line$
    push de
    ; Jump into next block
    ld de, #(BLOCK_JUMP_OFFSET - LINE_JUMP_OFFSET)
    add hl, de
    pop de
normal_line$:

    ; ENDFOR (loop over columns)
    dec c
    jr NZ, loop_lines$1

    ret


; Returns 1 if the current buffer is in 0xC000, 0 if 0x4000
; Args: -
; Ret: 1 or O (in A)
; Modifies: AF
_get_current_buffer:
    ld a, (#current_buffer)
    and #BUFFER_SWAP_XOR_MASK
    jr Z, ret$
    ld a, #1
ret$:
    ret


interrupt_service_routine:
    ; Backup registers
    ex af, af'
    exx

    ; Check if we are in VSync
    ld b, #VSYNC_IN
    in a, (c)
    rra
    jr NC, reti$

    ; Increase counter
    ld hl, #vsync_counter
    inc (hl)

    ; Restore registers and return
reti$:
    exx
    ex af', af
    ei
    reti


current_buffer:
    .db BUFFER_C000
vsync_counter:
    .db 0