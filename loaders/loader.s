.module loader
.globl _bg_dest_addr
.globl _code_dest_addr
.globl _init_dest_addr

.area _CODE

main:
    ld b, #(background_name_end-background_name)
    ld hl, #background_name
    ld de, #_bg_dest_addr
    call load_file

    ld b, #(code_name_end-code_name)
    ld hl, #code_name
    ld de, #_code_dest_addr
    call load_file

    ld b, #(init_name_end-init_name)
    ld hl, #init_name
    ld de, #_init_dest_addr
    call load_file

    call _init_dest_addr

load_error$:
    ret

; Load filename HL, filename size B
; Into DE
load_file:
    push de

    ; Print message
    push hl
    push hl
    ld hl, #loading_message
    call print_str
    pop hl
    call print_str

    ; CAS IN OPEN
    pop hl
    ld de, #0x0100
    call 0xBC77
    jr NC, load_error$

    ; CAS IN DIRECT
    pop hl
    call 0xBC83
    jr NC, load_error$

    ; CAS IN CLOSE
    call 0xBC7A
    jr NC, load_error$

    ret


; Print string pointed by HL
print_str:
    ld a, (hl)
    or a
    jr Z, ret$
    call #0xBB5A
    inc hl
    jr print_str
ret$:
    ret


background_name:
    .ascii "BACKGND.BIN"
background_name_end:
    .db 0x0D, 0x0A
    .db 0

code_name:
    .ascii "CODE.BIN"
code_name_end:
    .db 0x0D, 0x0A
    .db 0

init_name:
    .ascii "INIT.BIN"
init_name_end:
    .db 0x0D, 0x0A
    .db 0

loading_message:
    .ascii "Loading... "
    .db 0

.area _DATA