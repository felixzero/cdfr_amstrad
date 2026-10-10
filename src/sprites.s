.module sprites

; Exports
.globl _init_sprites
.globl _create_sprite
.globl _move_sprite
.globl _set_sprite_z_index
.globl _set_sprite_visibility
.globl _change_sprite_asset
.globl _trigger_sprite_redraw
.globl _draw_sprites

; Imports
.globl _get_current_buffer
.globl _blit_sprite_swap
.globl rect_merge_into
.globl rect_intersects
.globl _buffer_heap_fb1
.globl _buffer_heap_fb2


NUMBER_OF_BUFFERS = 2

STATUS_DISPLAYED_BIT = 0
STATUS_NEED_REDRAW_BIT = 1
STATUS_GRAPHICS_CHANGED_BIT = 2
STATUS_NO_ERASE = 0
STATUS_DISPLAYED_FLAG = (1 << STATUS_DISPLAYED_BIT)
STATUS_NEED_REDRAW_FLAG = (1 << STATUS_NEED_REDRAW_BIT)
STATUS_GRAPHICS_CHANGED = (1 << STATUS_GRAPHICS_CHANGED_BIT)

BLITTED_SPRITE_RECT = 0
BLITTED_SPRITE_SCREEN_BACKUP = 4
BLITTED_SPRITE_STATUS = 6
BLITTED_SPRITE_SIZEOF = 8

REQUESTED_SPRITE_GRAPHICS = 0
REQUESTED_SPRITE_RECT = 2
REQUESTED_SPRITE_ZINDEX = 6
REQUESTED_SPRITE_SIZE = 7
REQUESTED_SPRITE_SIZEOF = 8

MAX_NUMBER_OF_SPRITES = 40
SCREEN_HISTORY_SIZEOF = MAX_NUMBER_OF_SPRITES * (1 + BLITTED_SPRITE_SIZEOF)
SCREEN_HISTORY_BLITTED_SPRITES_OFFSET = MAX_NUMBER_OF_SPRITES


.area _CODE

_init_sprites:
    xor a
    ld (#allocated_size), a
    ld (#allocated_size + 1), a
    ld (#number_of_sprites), a
    ret


; Register a new sprite, setting graphics, size and position
;sprite_handle_t [A] create_sprite(const uint8_t *graphics [HL], struct rect *rect [DE])
_create_sprite:
    push ix
    push hl

    ; IX = rect;
    push de
    pop ix

    ; IY = &requested_sprites[number_of_sprites];
    ld a, (#number_of_sprites)
    ld l, a
    ld h, #0
    sla l
    sla l
    add hl, hl
    ld de, #requested_sprites
    add hl, de
    push hl
    pop iy

    ; request->graphics = graphics;
    pop hl
    ld REQUESTED_SPRITE_GRAPHICS(iy), l
    ld REQUESTED_SPRITE_GRAPHICS + 1(iy), h

    ; request->size = rect->w * rect->h / 2;
    ld a, #0
    ld b, 2(ix)
    srl b
repeat_add$:
    add a, 3(ix)
    djnz repeat_add$
    ld REQUESTED_SPRITE_SIZE(iy), a

    ; request->z_index = 0;
    ld REQUESTED_SPRITE_ZINDEX(iy), #0

    ; memcpy(&request->rect, rect, sizeof(struct rect));
    ld a, 0(ix)
    ld REQUESTED_SPRITE_RECT + 0(iy), a
    ld a, 1(ix)
    ld REQUESTED_SPRITE_RECT + 1(iy), a
    ld a, 2(ix)
    ld REQUESTED_SPRITE_RECT + 2(iy), a
    ld a, 3(ix)
    ld REQUESTED_SPRITE_RECT + 3(iy), a
    push iy

    ; current_heap = BUFFER_HEAP_FB1 + allocated_size;
    ld hl, (#allocated_size)
    ld de, (#_buffer_heap_fb1)
    add hl, de
    ld (#current_heap), hl

    ; current_history = &history[0];
    ld hl, #history
    ld (#current_history), hl
    call create_sprite_on_buffer

    ; current_heap = BUFFER_HEAP_FB2 + allocated_size;
    ld hl, (#allocated_size)
    ld de, (#_buffer_heap_fb2)
    add hl, de
    ld (#current_heap), hl

    ; current_history = &history[1];
    ld hl, #history
    ld de, #(MAX_NUMBER_OF_SPRITES * 9)
    add hl, de
    ld (#current_history), hl
    call create_sprite_on_buffer

    ; allocated_size += request->size;
    pop iy
    ld hl, (#allocated_size)
    ld d, #0
    ld e, REQUESTED_SPRITE_SIZE(iy)
    add hl, de
    ld (#allocated_size), hl

    ; requested_blit_order[number_of_sprites] = number_of_sprites;
    ld hl, #requested_blit_order
    ld a, (#number_of_sprites)
    ld e, a
    ld d, #0
    add hl, de
    ld (hl), a

    ; number_of_sprites++
    inc a
    ld (#number_of_sprites), a

    ld a, (#number_of_sprites)
    dec a

    pop ix
    ret


;void create_sprite_on_buffer(struct rect *rect [IX])
create_sprite_on_buffer:
    ; HL = &current_history->blitted_sprites[number_of_sprites]
    ld a, (#number_of_sprites)
    ld h, #0
    ld l, a
    sla l
    sla l
    add hl, hl
    ld de, (#current_history)
    add hl, de
    ld de, #MAX_NUMBER_OF_SPRITES
    add hl, de

    ; assume IY is blit
    push hl
    pop iy

    ; blit->rect.w = rect->w;
    ; blit->rect.h = rect->h;
    ld BLITTED_SPRITE_RECT + 0(iy), #0
    ld BLITTED_SPRITE_RECT + 1(iy), #0
    ld a, BLITTED_SPRITE_RECT + 2(ix)
    ld BLITTED_SPRITE_RECT + 2(iy), a
    ld a, BLITTED_SPRITE_RECT + 3(ix)
    ld BLITTED_SPRITE_RECT + 3(iy), a

    ; blit->status = STATUS_GRAPHICS_CHANGED;
    ld BLITTED_SPRITE_STATUS(iy), #STATUS_GRAPHICS_CHANGED

    ; blit->screen_backup = current_heap;
    ld a, (#current_heap)
    ld BLITTED_SPRITE_SCREEN_BACKUP(iy), a
    ld a, (#current_heap + 1)
    ld BLITTED_SPRITE_SCREEN_BACKUP + 1(iy), a

    ; current_history->blit_order[number_of_sprites] = number_of_sprites;
    ld a, (#number_of_sprites)
    ld l, a
    ld h, #0
    ld de, (#current_history)
    add hl, de
    ld (hl), a

    ret


; Change sprite position
; void move_sprite(sprite_handle_t sprite [A], struct point *position [DE])
_move_sprite:
    push de

    ; DE = &requested_sprites[sprite].rect.x
    sla a
    sla a
    ld h, #0
    ld l, a
    add hl, hl
    ld de, #requested_sprites
    add hl, de
    ld de, #REQUESTED_SPRITE_RECT
    add hl, de
    push hl
    pop de

    ; requested_sprites[sprite].rect.x = position->x;
    ; requested_sprites[sprite].rect.y = position->y;
    pop hl
    ldi
    ldi

    ret


; Update sprite z_index
; If z_index < 0, the sprite is not displayed
; void set_sprite_z_index(sprite_handle_t sprite [A], int8_t z_index [L])
_set_sprite_z_index:
    push hl

    ; DE = &requested_sprites[sprite].z_index
    sla a
    sla a
    ld h, #0
    ld l, a
    add hl, hl
    ld de, #requested_sprites
    add hl, de
    ld de, #REQUESTED_SPRITE_ZINDEX
    add hl, de

    ; requested_sprites[sprite].z_index = z_index;
    pop de
    ld a, e
    ld (hl), a

    ret


; Hide or display the sprite
; void set_sprite_visibility(sprite_handle_t sprite [A], bool visible [L])
_set_sprite_visibility:
    push hl

    ; HL = &requested_sprites[sprite].z_index
    sla a
    sla a
    ld h, #0
    ld l, a
    add hl, hl
    ld de, #requested_sprites
    add hl, de
    ld de, #REQUESTED_SPRITE_ZINDEX
    add hl, de

    pop de
    bit 0, e
    jr NZ, visible$

    set 7, (hl)
    jr ret$
visible$:
    res 7, (hl)

ret$:
    ret


; Flag the sprite for redraw, as well as other sprites intersecting it
; void trigger_sprite_redraw(sprite_handle_t sprite [A])
_trigger_sprite_redraw:
    ld l, a
    push hl
    xor a
    call trigger_sprite_redraw_buffer
    pop hl
    ld a, #1
;void trigger_sprite_redraw_buffer(uint8_t buffer [A], sprite_handle_t sprite [L])
trigger_sprite_redraw_buffer:
    push ix

    ; IY = &requested_sprites[sprite].rect;
    ld iy, #requested_sprites
    ld h, #0
    sla l
    sla l
    add hl, hl
    push hl
    ld de, #REQUESTED_SPRITE_RECT
    add hl, de
    push hl
    pop de
    add iy, de

    ; buffer_history = &history[buffer];
    ld hl, #history
    or a, a
    jr Z, buffer_0$
    ld de, #(MAX_NUMBER_OF_SPRITES * (1 + BLITTED_SPRITE_SIZEOF))
    add hl, de
buffer_0$:
    ld (#current_history), hl

    ; top of stack = &buffer_history->blitted_sprites[sprite].rect;
    ld de, #MAX_NUMBER_OF_SPRITES
    add hl, de
    pop de
    add hl, de
    push hl

    ; IX = &r
    ld ix, #redraw_bounding_box

    ; memcpy(&r, &requested_sprites[sprite].rect, sizeof(struct rect));
    ld a, 0(iy)
    ld 0(ix), a
    ld a, 1(iy)
    ld 1(ix), a
    ld a, 2(iy)
    ld 2(ix), a
    ld a, 3(iy)
    ld 3(ix), a

    ; rect_merge_into(&r, &buffer_history->blitted_sprites[sprite].rect);
    pop iy
    call rect_merge_into

    ; B = number_of_sprites
    ld hl, #number_of_sprites
    ld b, (hl)

    ; IY = buffer_history->blitted_sprites
    ld iy, (#current_history)
    ld de, #MAX_NUMBER_OF_SPRITES
    add iy, de

loop_over_sprites$:
    push bc

    ; if (rect_intersects(&r, &blit->rect)) 
    call rect_intersects
    jr C, skip_need_redraw$

    ; blit->status |= STATUS_NEED_REDRAW_FLAG;
    set STATUS_NEED_REDRAW_BIT, BLITTED_SPRITE_STATUS(iy)

    ; rect_merge_into(&r, &blit->rect);
    call rect_merge_into
skip_need_redraw$:

    ; IY += sizeof(struct blitted_sprite)
    ld de, #BLITTED_SPRITE_SIZEOF
    add iy, de
    
    ; WHILE b > 0
    pop bc
    djnz loop_over_sprites$

    pop ix

    ret


; Change the graphics of a sprite, to another one of the same dimensions
; void change_sprite_asset(sprite_handle_t sprite [HL], const uint8_t *graphics [DE])
_change_sprite_asset:
    push de

    ; HL = &requested_sprites[sprite].graphics
    sla a
    sla a
    ld h, #0
    ld l, a
    add hl, hl
    push hl
    pop iy
    ld de, #requested_sprites
    add hl, de

    ; requested_sprites[sprite].graphics = graphics;
    pop de
    ld (hl), e
    inc hl
    ld (hl), d

    ; history[0].blitted_sprites[sprite].status |= STATUS_GRAPHICS_CHANGED;
    ; history[1].blitted_sprites[sprite].status |= STATUS_GRAPHICS_CHANGED;
    push iy
    pop hl
    ld de, #(history + MAX_NUMBER_OF_SPRITES + BLITTED_SPRITE_STATUS)
    add hl, de
    set STATUS_GRAPHICS_CHANGED_BIT, (hl)
    ld de, #(MAX_NUMBER_OF_SPRITES * 9)
    add hl, de
    set STATUS_GRAPHICS_CHANGED_BIT, (hl)

    ret


; Update display of sprites
; void draw_sprites(void);
_draw_sprites:
    ; currentcurrent_history = &history[get_current_buffer()];
    call _get_current_buffer
    ld hl, #history
    bit 0, a
    jr Z, skip_buffer_change$
    ld de, #SCREEN_HISTORY_SIZEOF
    add hl, de
skip_buffer_change$:
    ld (#current_history), hl

    ; == First undo the elements that need to be redrawn, in reverse blitting order
    ; FOR(C = number_of_sprites - 1; C >= 0; --C)
    ld b, #0
    ld a, (#number_of_sprites)
    dec a
    ld c, a

loop_clear$:
    ; A = currentcurrent_history->blit_order[c]
    ld hl, (#current_history)
    add hl, bc
    ld a, (hl)

    ; IY = &currentcurrent_history->blitted_sprites[A]
    ld h, #0
    ld l, a
    sla l
    sla l
    add hl, hl
    ld de, (#current_history)
    add hl, de
    ld de, #(MAX_NUMBER_OF_SPRITES)
    add hl, de
    push hl
    pop iy

    ; if ((blit->status & STATUS_NEED_REDRAW_FLAG) && (blit->status & STATUS_DISPLAYED_FLAG)), skip
    ld a, BLITTED_SPRITE_STATUS(iy)
    bit STATUS_DISPLAYED_BIT, a
    jr Z, skip_erase$
    bit STATUS_NEED_REDRAW_BIT, a
    jr Z, skip_erase$

    ; Unblit
    push iy
    ld h, BLITTED_SPRITE_SCREEN_BACKUP+1(iy)
    ld l, BLITTED_SPRITE_SCREEN_BACKUP(iy)
    push iy
    pop de
    push bc
    call _blit_sprite_swap
    pop bc
    pop iy

skip_erase$:
    dec c
    jp P, loop_clear$

    ; == Reorder sprites by z-index
    ; FOR (c = 1; c < number_of_sprites; ++c)
    ld c, #1

outer_loop$:
    ; E = requested_blit_order[c]
    ; HL = requested_blit_order + c
    ld hl, #requested_blit_order
    ld b, #0
    add hl, bc
    ld e, (hl)

    ; D = requested_sprites[E].z_index
    push hl
    push de
    ld h, #0
    ld l, e
    sla l
    sla l
    add hl, hl
    ld de, #(requested_sprites + REQUESTED_SPRITE_ZINDEX)
    add hl, de
    pop de
    ld d, (hl)
    pop hl

    ; FOR (b = c; b > 0; b--)
    ld b, c

inner_loop$:
    ; A = requested_blit_order[b - 1]
    dec hl
    ld a, (hl)

    ; A = requested_sprites[A].z_index
    push hl
    push de
    ld h, #0
    ld l, a
    sla l
    sla l
    add hl, hl
    ld de, #(requested_sprites + REQUESTED_SPRITE_ZINDEX)
    add hl, de
    ld a, (hl)
    pop de
    pop hl

    ; if A <= D, exit loop
    cp a, d
    jr C, exit_inner_loop$
    jr Z, exit_inner_loop$

    ; requested_blit_order[b] = requested_blit_order[b - 1]
    ld a, (hl)
    inc hl
    ld (hl), a
    dec hl

    ; ENDFOR
    djnz inner_loop$
    dec hl
exit_inner_loop$:
    inc hl

    ; requested_blit_order[b] = e;
    ld (hl), e

    inc c
    ld a, c
    ld hl, #number_of_sprites
    cp a, (hl)
    jr NZ, outer_loop$

    ; == Then we draw the updated elements
    ; IX is callee-saved
    push ix

    ; FOR (c = 0; c < number_of_sprites; ++c)
    ld b, #0
    ld c, #0
for_loop_over_sprites$:
    ; A = requested_blit_order[i]
    ld hl, #requested_blit_order
    add hl, bc
    ld a, (hl)
    push af

    ; IY = &currentcurrent_history->blitted_sprites[effective]
    ld h, #0
    ld l, a
    sla l
    sla l
    add hl, hl
    push hl
    ld de, (#current_history)
    add hl, de
    ld de, #(MAX_NUMBER_OF_SPRITES)
    add hl, de
    push hl
    pop iy

    ; IX = &requested_sprites[effective]
    pop hl
    ld de, #requested_sprites
    add hl, de
    push hl
    pop ix

    push bc

    ; IX = request
    ; IY = blit
    ; IF blit->status not need redraw, skip
    ld a, BLITTED_SPRITE_STATUS(iy)
    bit STATUS_NEED_REDRAW_BIT, a
    jr Z, skip_painting$

    ; IF blit->status not changed graphics, skip
    bit STATUS_GRAPHICS_CHANGED_BIT, a
    jr Z, skip_graphics_changed$

    ; MEMCPY blit->screen_backup, request->graphics, request->size
    ld h, REQUESTED_SPRITE_GRAPHICS+1(ix)
    ld l, REQUESTED_SPRITE_GRAPHICS(ix)
    ld d, BLITTED_SPRITE_SCREEN_BACKUP+1(iy)
    ld e, BLITTED_SPRITE_SCREEN_BACKUP(iy)
    ld b, #0
    ld c, REQUESTED_SPRITE_SIZE(ix)
    ldir
skip_graphics_changed$:

    ; IF request->z_index < 0, hide the sprite
    ld a, REQUESTED_SPRITE_ZINDEX(ix)
    bit 7, a
    jr NZ, hidden_sprite$

    ; Blit !
    push iy
    ld h, BLITTED_SPRITE_SCREEN_BACKUP+1(iy)
    ld l, BLITTED_SPRITE_SCREEN_BACKUP(iy)
    push ix
    pop de
    inc de
    inc de
    call _blit_sprite_swap
    pop iy

    ; Mark as displayed
    ld BLITTED_SPRITE_STATUS(iy), #STATUS_DISPLAYED_FLAG

    ; Copy request rect into blit
    push ix
    pop hl
    inc hl
    inc hl
    push iy
    pop de
    ld b, #0
    ld c, #4
    ldir
    jr skip_painting$

    ; Mark hidden sprite as not displayed
hidden_sprite$:
    ld BLITTED_SPRITE_STATUS(iy), #0

skip_painting$:

    ; currentcurrent_history->blit_order[i] = effective;
    pop bc
    pop af
    ld hl, (#current_history)
    add hl, bc
    ld (hl), a

    inc c
    ld a, (#number_of_sprites)
    cp a, c
    jp NZ, for_loop_over_sprites$

    pop ix
    ret


.area _DATA
number_of_sprites:
    .ds 1
allocated_size:
    .ds 2
requested_sprites:
    .ds (MAX_NUMBER_OF_SPRITES * REQUESTED_SPRITE_SIZEOF)
history:
    .ds (NUMBER_OF_BUFFERS * SCREEN_HISTORY_SIZEOF)
requested_blit_order:
    .ds MAX_NUMBER_OF_SPRITES

; Local variables
current_history:
    .ds 2
current_heap:
    .ds 2
redraw_bounding_box:
    .ds 4
