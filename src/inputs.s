.globl _get_keypress

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

.macro read_line line, outreg
    ; Select line to read through PSG
    ld b, #(PPI_PORT_C >> 8)
    ld a, #line
    out (c), a
    ; Read port A
    ld b, #(PPI_PORT_A >> 8)
    in a, (c)
    ld outreg, a
.endm

; Returns a bit flag with pressed keys
; Args: -
; Ret: HL-DE = Key press map
; Affects: AF, BC, DE, HL
_get_keypress:
    ;rst #0x30

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

    read_line QAESC_LINE, h
    read_line DSZ_LINE, l
    read_line LCPY_LINE, d
    read_line UDR_LINE, e

    ; Set port A in output mode
    ld bc, #(PPI_CTRL | 0x82)
    out (c), c

    ; Reactivate PSG
    dec b ; PPI_PORT_C
    xor a
    out (c), a

    ret
