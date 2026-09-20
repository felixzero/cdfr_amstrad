#pragma once

#include <stdint.h>

#define KEY_UP      (1L << 0)
#define KEY_RIGHT   (1L << 1)
#define KEY_DOWN    (1L << 2)
#define KEY_LEFT    (1L << 8)
#define KEY_COPY    (1L << 9)
#define KEY_Z       (1L << 19)
#define KEY_S       (1L << 20)
#define KEY_D       (1L << 21)
#define KEY_ESC     (1L << 26)
#define KEY_A       (1L << 27)
#define KEY_Q       (1L << 29)

// Returns a bit flag with pressed keys
uint32_t get_keypress(void);
