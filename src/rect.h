#pragma once

// (0, 0) is the top left corner of the screen
struct point {
    uint8_t x;
    uint8_t y;
};

// (x, y) defines the top-left corner of the rectangle
// (w, h) its dimension
struct rect {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
};

bool rect_contains(const struct rect *r1, const struct point *p);

bool rect_intersects(const struct rect *r1, const struct rect *r2);

void rect_merge_into(struct rect *r1, const struct rect *r2);
