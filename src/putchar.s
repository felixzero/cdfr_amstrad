.globl _putchar

.area _CODE

_putchar:
  ld a, l
  call 0xBB5A
  ld de, #00
  ret
