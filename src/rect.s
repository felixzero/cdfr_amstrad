.globl _rect_contains
.globl _rect_intersects
.globl _rect_merge_into

.globl rect_intersects
.globl rect_merge_into

POINT_X = 0
POINT_Y = 1

RECT_X = 0
RECT_Y = 1
RECT_W = 2
RECT_H = 3

; bool rect_contains(const struct rect *r1, const struct point *p);
; Returns 1 if r1 contains p
; Args: r1 in HL/IX, p in DE/IY
; Ret: 1 or O (in A)
; Modifies: AF, HL, DE, IY
_rect_contains:
    push ix
    push hl
    pop ix
    push de
    pop iy

    ; p.x - r.x < 0
    ld a, POINT_X(iy)
    sub a, RECT_X(ix)
    jp M, false$0

    ; r.x + r.w < p.x
    ld a, RECT_X(ix)
    add a, RECT_W(ix)
    cp a, POINT_X(iy)
    jr C, false$0

    ; p.y - r.y < 0
    ld a, POINT_Y(iy)
    sub a, RECT_Y(ix)
    jp M, false$0

    ; r.y + r.h < p.y
    ld a, RECT_Y(ix)
    add a, RECT_H(ix)
    cp a, POINT_Y(iy)
    jr C, false$0
    
    pop ix
    ld a, #1
    ret

false$0:
    pop ix
    xor a
    ret


; bool rect_intersects(const struct rect *r1, const struct rect *r2);
; Returns 1 if r1 and r2 intersect
_rect_intersects:
    push ix
    push hl
    pop ix
    push de
    pop iy

    call rect_intersects
    jr C, false$

    pop ix
    ld a, #1
    ret

false$:
    pop ix
    xor a
    ret


; Returns 1 if r1 and r2 intersect
; Args: r1 in IX, r2 in IY
; Ret: C=1 if no intersect, else C=0
; Modifies: AF
rect_intersects:
    ; r1.x + r1.w < r2.x
    ld a, RECT_X(ix)
    add RECT_W(ix)
    cp RECT_X(iy)
    jr C, no_intersect$

    ; r2.x + r2.w < r1.x
    ld a, RECT_X(iy)
    add RECT_W(iy)
    cp RECT_X(ix)
    jr C, no_intersect$

    ; r1.y + r1.h < r2.y
    ld a, RECT_Y(ix)
    add RECT_H(ix)
    cp RECT_Y(iy)
    jr C, no_intersect$

    ; r2.y + r2.h < r1.y
    ld a, RECT_Y(iy)
    add RECT_H(iy)
    cp RECT_Y(ix)
    jr C, no_intersect$

no_intersect$:
    ret


; void rect_merge_into(struct rect *r1, const struct rect *r2);
; Compute the bounding rectangle including r1 and r2, saving the result into r1
_rect_merge_into:
    push ix
    push hl
    pop ix
    push de
    pop iy

    call rect_merge_into

    pop ix
    ret


; rect_merge_into (direct assembly version)
; Compute the bounding rectangle including r1 and r2, saving the result into r1
; Args: r1 in IX, r2 in IY
; Ret: -
; Modifies: AF, BC
rect_merge_into:
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
    jr C, a$
    ld a, RECT_X(iy)
a$:
    ; r1->x = A
    ld RECT_X(ix), a
    ; r1->w = (-A) + B
    neg
    add a, b
    ld RECT_W(ix), a

    ; r1->y = min(r1->y, r2->y);
    ; r1->h = max(r1->y + r1->h, r2->y + r2->h) - min(r1->y, r2->y);
    ; B = max(r1->y + r1->h, r2->y + r2->h)
    ld a, RECT_Y(ix)
    add RECT_H(ix)
    ld b, a
    ld a, RECT_Y(iy)
    add RECT_H(iy)
    cp a, b
    jr C, max_bottom$
    ld b, a
max_bottom$:
    ; A = min(r1->y, r2->y)
    ld a, RECT_Y(ix)
    cp a, RECT_Y(iy)
    jr C, b$
    ld a, RECT_Y(iy)
b$:
    ; r1->y = A
    ld RECT_Y(ix), a
    ; r1->h = (-A) + B
    neg
    add a, b
    ld RECT_H(ix), a

    ret
