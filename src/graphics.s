.globl _wait_for_vsync
.globl _swap_buffers
.globl _set_palette
.globl _blit_sprite_swap
.globl _get_current_buffer

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

; Block execution until the beginning of VSync
; Args: -
; Ret: -
; Modifies: AF, BC
_wait_for_vsync:
    ld b, #VSYNC_IN
wait_loop$:
    in a, (c)
    rra
    jr NC, wait_loop$
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
    ex af, af'
    ld a, (hl)
    ld (de), a
    ex af, af'
    ld (hl), a
skip_trans$:
    inc hl
    inc de
    djnz loop_columns$1

    ; Calculate next line
    ; First restore beginning of line
    pop hl
    ; Next line = line + 0x0800
    ld a, h
    add #0x08
    ld h, a
    ; If bit 6 is 1, still inside the screen
    bit 6, h
    jr NZ, normal_line$
    push de
    ; Jump into next block
    ld de, #(-0xF800 + 0xC050 - 0x0800)
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


.area _INITIALIZED
current_buffer:
    .db BUFFER_C000
