.module crt0
.globl	_main

.area _DATA

.area _INIT (ABS)
    ld sp, #0xBFFF
    di
    call _main

.area _CODE
.area _INITIALIZED
