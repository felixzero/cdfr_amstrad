.module crt0
.globl	_main

.area _DATA

.area _INIT (ABS)
    ; No interrupts
    di
    ; Debank ROM, if needed
    ld bc, #0x7F8C
    out (c), c
    ; Stack pointer at top of usable memory
    ld sp, #0xBFFF
    ; Run program
    call _main

.area _CODE
.area _INITIALIZED
