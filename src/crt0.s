.module crt0
.globl	_main

.area _DATA

.area _CODE

.area _INIT (ABS)
    ; No interrupts
    di
    ; Debank ROM, if needed
    ld bc, #0x7F8C
    out (c), c
    jp _main

.area _INITIALIZED
