.module crt0
.globl	_main

.area _DATA

.area _INIT (ABS)
  di
  call _main

_putchar:
  ld a, l
  call 0xBB5A
  ld de, #00
  ret

.area _CODE
.area _INITIALIZED
