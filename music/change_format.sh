/opt/rasm $1.asm -o $1 -s -sl -sq
/opt/disark $1.bin $1.s --symbolFile $1.sym --sourceProfile sdcc
