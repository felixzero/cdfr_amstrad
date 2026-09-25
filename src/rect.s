.globl _rect_intersects
.globl _rect_merge_into

RECT_X = 0
RECT_Y = 1
RECT_W = 2
RECT_H = 3

; bool rect_intersects(const struct rect *r1, const struct rect *r2);
; r1 = HL ; r2 = DE
_rect_intersects:
    push ix
    push hl
    pop ix
    push de
    pop iy

    ld a, RECT_X(ix)
    add RECT_W(ix)
    cp RECT_X(iy)
    jr C, false$

    ld a, RECT_X(iy)
    add RECT_W(iy)
    cp RECT_X(ix)
    jr C, false$

    ld a, RECT_Y(iy)
    add RECT_H(ix)
    inc a
    cp RECT_Y(ix)
    jr C, false$

    ld a, RECT_Y(ix)
    add RECT_H(iy)
    inc a
    cp RECT_Y(iy)
    jr C, false$

    pop ix
    ld a, #1
    ret

false$:
    pop ix
    xor a
    ret

; void rect_merge_into(struct rect *r1, const struct rect *r2);
_rect_merge_into:
    push ix
    push hl
    pop ix
    push de
    pop iy

    ; r1->x = min(r1->x, r2->x);
    ; r1->w = max(r1->x + r1->w, r2->x + r2->w) - min(r1->x, r2->x);
    ; B = max(r1->x + r1->w, r2->x + r2->w)
    ld a, RECT_X(ix)
    add RECT_W(ix)
    ld b, a
    ld a, RECT_X(iy)
    add RECT_W(iy)
    cp a, b
    jr C, max_right$
    ld b, a
max_right$:
    ; A = min(r1->x, r2->x)
    ld a, RECT_X(ix)
    cp a, RECT_X(iy)
    jr C, w$
    ld a, RECT_X(iy)
w$:
    ; r1->x = A
    ld RECT_X(ix), a
    ; r1->w = (-A) + B
    neg
    add a, b
    ld RECT_W(ix), a

    ; r1->y = max(r1->y, r2->y);
    ; r1->h = max(r1->y, r2->y) - min(r1->y - r1->h, r2->y - r2->h);
    ; B = min(r1->y - r1->h, r2->y - r2->h)
    ld a, RECT_Y(ix)
    sub RECT_H(ix)
    ld b, a
    ld a, RECT_Y(iy)
    sub RECT_H(iy)
    cp a, b
    jr NC, min_top$
    ld b, a
min_top$:
    ; A = max(r1->y, r2->y)
    ld a, RECT_Y(ix)
    cp a, RECT_Y(iy)
    jr NC, h$
    ld a, RECT_Y(iy)
h$:
    ; r1->y = A
    ld RECT_Y(ix), a
    ; r1->h = A - B
    sub a, b
    ld RECT_H(ix), a

    pop ix
    ret
