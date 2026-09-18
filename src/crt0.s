.module crt0
.globl	_main

.area _CODE
  call gsinit
  di

  ; Set mode 0
  ld bc, #0x7F8C
  out (c), c

  call _main
  jp _exit

  .area _GSINIT
  .area _GSFINAL
        
	.area	_DATA
  .area _BSS
  .area _HEAP

  .area _CODE
__clock::
	ret
	
_exit::
	ret
	
	.area  _GSINIT
gsinit::	

.area  _GSFINAL
  ret
