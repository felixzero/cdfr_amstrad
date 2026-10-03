.module crt0
.globl _main
.globl _init
.globl gsinit
.globl gsfinal
.globl l__INITIALIZED
.globl s__INITIALIZED
.globl s__INITIALIZER

.area _CODE

.area _GSINIT
gsinit::
    ld bc, #l__INITIALIZED

    ; Skip if size is zero
    ld a, b
    or c
    jr Z, gsfinal

    ld de, #s__INITIALIZED
    ld hl, #s__INITIALIZER
    ldir
.area _GSFINAL
gsfinal::
    ret

.area _INIT (ABS)
_init:
    ; No interrupts
    di
    ; Debank ROM, if needed
    ld bc, #0x7F8C
    out (c), c
    ; Call initializer
    call gsinit
    jp _main

.area _INITIALIZED
.area _SPRITE_ASSETS
.area _CODE_ASSETS
.area _INITIALIZER

.area _DATA