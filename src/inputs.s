.globl _get_keypress
.globl _check_key_up
.globl _check_key_down
.globl _check_key_left
.globl _check_key_right
.globl _check_key_action
.globl _is_key_pressed

PPI_PORT_A = 0xF400
PPI_PORT_B = 0xF500
PPI_PORT_C = 0xF600
PPI_CTRL   = 0xF700

PSG_KEYBOARD_REG = 0x0E
PSG_SEL_REG = 0xC0

QAESC_LINE = 0x48
DSZ_LINE = 0x47
LCPY_LINE = 0x41
UDR_LINE = 0x40

.area _CODE

.macro read_line line, outvar
    ; Select line to read through PSG
    ld b, #(PPI_PORT_C >> 8)
    ld a, #line
    out (c), a
    ; Read port A
    ld b, #(PPI_PORT_A >> 8)
    in a, (c)
    ld (#outvar), a
.endm

; Returns a bit flag with pressed keys
; Args: -
; Ret: HL-DE = Key press map
; Affects: AF, BC, DE, HL
_get_keypress:
    ; Configure PSG (sound generator) to activate its keyboard I/O
    ld bc, #(PPI_PORT_A | PSG_KEYBOARD_REG)
    out (c), c
    ld bc, #(PPI_PORT_C | PSG_SEL_REG)
    out (c), c
    xor a
    out (c), a

    ; Set port A in input mode
    ld bc, #(PPI_CTRL | 0x92)
    out (c), c

    read_line QAESC_LINE, qaesc_line
    read_line DSZ_LINE, dsz_line
    read_line LCPY_LINE, lcpy_line
    read_line UDR_LINE, udr_line

    ; Set port A in output mode
    ld bc, #(PPI_CTRL | 0x82)
    out (c), c

    ; Reactivate PSG
    dec b ; PPI_PORT_C
    xor a
    out (c), a

    ret


_is_key_pressed:
    ld h, #0
    ld l, a
    xor a
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, de
    jp (hl)
ret_key_pressed:
    jr NZ, skip$
    ld a, #1
skip$:
    ret


_check_key_up:
    ld hl, #dsz_line
    bit 3, (hl)
    jp ret_key_pressed
    ld hl, #udr_line
    bit 0, (hl)
    jp ret_key_pressed
_check_key_down:
    ld hl, #dsz_line
    bit 4, (hl)
    jp ret_key_pressed
    ld hl, #udr_line
    bit 2, (hl)
    jp ret_key_pressed
_check_key_left:
    ld hl, #qaesc_line
    bit 5, (hl)
    jp ret_key_pressed
    ld hl, #lcpy_line
    bit 0, (hl)
    jp ret_key_pressed
_check_key_right:
    ld hl, #dsz_line
    bit 5, (hl)
    jp ret_key_pressed
    ld hl, #udr_line
    bit 1, (hl)
    jp ret_key_pressed
_check_key_action:
    ld hl, #qaesc_line
    bit 3, (hl)
    jp ret_key_pressed
    ld hl, #lcpy_line
    bit 1, (hl)
    jp ret_key_pressed


.area _DATA
qaesc_line:
    .ds 1
dsz_line:
    .ds 1
lcpy_line:
    .ds 1
udr_line:
    .ds 1
