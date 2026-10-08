#define MAX_NUMBER_OF_SPRITES 40

#include "sprites.h"
#include "sprite_assets.h"
#include <string.h>
#include <stddef.h>
#include <assert.h>

#define STATUS_DISPLAYED_BIT 0
#define STATUS_NEED_REDRAW_BIT 1
#define STATUS_GRAPHICS_CHANGED_BIT 2
#define STATUS_NO_ERASE         0
#define STATUS_DISPLAYED_FLAG   (1 << STATUS_DISPLAYED_BIT)
#define STATUS_NEED_REDRAW_FLAG (1 << STATUS_NEED_REDRAW_BIT)
#define STATUS_GRAPHICS_CHANGED (1 << STATUS_GRAPHICS_CHANGED_BIT)

struct blitted_sprite {
    struct rect rect;
    const uint8_t *screen_backup;
    uint8_t status;
    uint8_t __padding;
};
#define BLITTED_SPRITE_RECT 0
#define BLITTED_SPRITE_SCREEN_BACKUP 4
#define BLITTED_SPRITE_STATUS 6

struct screen_history
{
    sprite_handle_t blit_order[MAX_NUMBER_OF_SPRITES];
    struct blitted_sprite blitted_sprites[MAX_NUMBER_OF_SPRITES];
};

struct requested_sprite
{
    const uint8_t *graphics;
    struct rect rect;
    int8_t z_index;
    uint8_t size;
};
#define REQUESTED_SPRITE_GRAPHICS 0
#define REQUESTED_SPRITE_RECT 2
#define REQUESTED_SPRITE_ZINDEX 6
#define REQUESTED_SPRITE_SIZE 7

static struct requested_sprite requested_sprites[MAX_NUMBER_OF_SPRITES];

static struct screen_history history[NUMBER_OF_BUFFERS];
static sprite_handle_t requested_blit_order[MAX_NUMBER_OF_SPRITES];
static uint8_t number_of_sprites = 0;

#define BUFFER_HEAP_FB1 ((uint8_t*)(0x4000 - FB_BUFFER_HEAP_SIZE))
#define BUFFER_HEAP_FB2 ((uint8_t*)(0xC000 - FB_BUFFER_HEAP_SIZE))
static uint16_t allocated_size = 0;


sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *rect)
{
    static uint8_t i;
    static struct blitted_sprite *blit;

    requested_sprites[number_of_sprites].size = rect->w * rect->h / 2;

    requested_sprites[number_of_sprites].graphics = graphics;
    memcpy(&requested_sprites[number_of_sprites].rect, rect, sizeof(struct rect));
    requested_sprites[number_of_sprites].z_index = 0;

    for (i = 0; i < NUMBER_OF_BUFFERS; ++i) {
        blit = &history[i].blitted_sprites[number_of_sprites];

        memset(blit, 0, sizeof(struct blitted_sprite));
        blit->status |= STATUS_GRAPHICS_CHANGED;
        blit->screen_backup = (i == 0) ? BUFFER_HEAP_FB1 + allocated_size : BUFFER_HEAP_FB2 + allocated_size;
        blit->rect.w = rect->w;
        blit->rect.h = rect->h;
        history[i].blit_order[number_of_sprites] = number_of_sprites;
    }
    allocated_size += requested_sprites[number_of_sprites].size;

    requested_blit_order[number_of_sprites] = number_of_sprites;

    return number_of_sprites++;
}


void move_sprite(sprite_handle_t sprite, struct point *position)
{
    requested_sprites[sprite].rect.x = position->x;
    requested_sprites[sprite].rect.y = position->y;
}


void set_sprite_z_index(sprite_handle_t sprite, int8_t z_index)
{
    requested_sprites[sprite].z_index = z_index;
}


void set_sprite_visibility(sprite_handle_t sprite, bool visible)
{
    if ((requested_sprites[sprite].z_index >= 0) != visible) {
        requested_sprites[sprite].z_index = -requested_sprites[sprite].z_index;
    }
}


void change_sprite_asset(sprite_handle_t sprite, const uint8_t *graphics)
{
    requested_sprites[sprite].graphics = graphics;
    history[0].blitted_sprites[sprite].status |= STATUS_GRAPHICS_CHANGED;
    history[1].blitted_sprites[sprite].status |= STATUS_GRAPHICS_CHANGED;
}


void trigger_sprite_redraw(sprite_handle_t sprite)
{
    static struct rect r;
    static uint8_t buffer;
    static sprite_handle_t i, effective;
    static struct screen_history *buffer_history;
    static struct blitted_sprite *blit;

    for (buffer = 0; buffer < NUMBER_OF_BUFFERS; ++buffer) {
        buffer_history = &history[buffer];
        memcpy(&r, &requested_sprites[sprite].rect, sizeof(struct rect));

        rect_merge_into(&r, &buffer_history->blitted_sprites[sprite].rect);

        for (i = 0; i < number_of_sprites; ++i) {
            effective = buffer_history->blit_order[i];
            blit = &buffer_history->blitted_sprites[effective];

            if (rect_intersects(&r, &blit->rect)) {
                blit->status |= STATUS_NEED_REDRAW_FLAG;
                rect_merge_into(&r, &blit->rect);
            }
        }
    }
}


struct screen_history *current_buffer_history;
void draw_sprites(void)
{
    static uint8_t current_buffer;
    static sprite_handle_t i, j, effective;
    static struct blitted_sprite* blit;
    static struct requested_sprite *request;

    current_buffer = get_current_buffer();
    current_buffer_history = &history[current_buffer];

    // First undo the elements that need to be redrawn, in reverse blitting order
    __asm
    ; FOR(C = number_of_sprites - 1; C >= 0; --C)
    ld b, #0
    ld a, (#_number_of_sprites)
    dec a
    ld c, a

loop_clear$:
    ; A = current_buffer_history->blit_order[c]
    ld hl, (#_current_buffer_history)
    add hl, bc
    ld a, (hl)

    ; IY = &current_buffer_history->blitted_sprites[A]
    ld h, #0
    ld l, a
    sla l
    sla l
    add hl, hl
    ld de, (#_current_buffer_history)
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
    __endasm;

    // Reorder sprites by z-index
    __asm
    ; FOR (c = 1; c < number_of_sprites; ++c)
    ld c, #1

outer_loop$:
    ; E = requested_blit_order[c]
    ; HL = requested_blit_order + c
    ld hl, #_requested_blit_order
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
    ld de, #REQUESTED_SPRITE_ZINDEX
    add hl, de
    ld de, #_requested_sprites
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
    ld de, #_requested_sprites
    add hl, de
    ld de, #REQUESTED_SPRITE_ZINDEX
    add hl, de
    ld a, (hl)
    pop de
    pop hl

    ; if A < D, exit loop
    cp a, d
    jr C, exit_inner_loop$

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
    ld hl, #_number_of_sprites
    cp a, (hl)
    jr NZ, outer_loop$
__endasm;

    // Then we draw the updated elements
__asm
    ; IX is callee-saved
    push ix

    ; FOR (c = 0; c < number_of_sprites; ++c)
    ld b, #0
    ld c, #0
for_loop_over_sprites$:
    ; A = requested_blit_order[i]
    ld hl, #_requested_blit_order
    add hl, bc
    ld a, (hl)
    push af

    ; IY = &current_buffer_history->blitted_sprites[effective]
    ld h, #0
    ld l, a
    sla l
    sla l
    add hl, hl
    push hl
    ld de, (#_current_buffer_history)
    add hl, de
    ld de, #(MAX_NUMBER_OF_SPRITES)
    add hl, de
    push hl
    pop iy

    ; IX = &requested_sprites[effective]
    pop hl
    ld de, #_requested_sprites
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

    ; current_buffer_history->blit_order[i] = effective;
    pop bc
    pop af
    ld hl, (#_current_buffer_history)
    add hl, bc
    ld (hl), a

    inc c
    ld a, (#_number_of_sprites)
    cp a, c
    jp NZ, for_loop_over_sprites$

    pop ix
__endasm;
}
