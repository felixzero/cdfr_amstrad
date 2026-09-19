#pragma once

#include <stdint.h>

struct point {
    uint8_t x;
    uint8_t y;
};

struct rect {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
};

void swap_buffers(void);

void wait_for_vsync(void);

void blit_sprite_xor(const uint8_t *sprite, const struct rect *position);
