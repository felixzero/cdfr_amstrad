#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "rect.h"

// Block execution until the beginning of VSync
void swap_buffers(void);

// Swap the currently displayed buffer with the work buffer
void wait_for_vsync(void);

// Pick an indexed color as a palette element
void set_palette(uint8_t index, uint8_t value);

// Blit or unblit a sprite onto the work buffer, swapping the content of the buffer with the sprite asset
void blit_sprite_swap(const uint8_t *sprite, const struct rect *position);

// Returns 1 if the current buffer is in 0xC000, 0 if 0x4000
uint8_t get_current_buffer(void);

// Amstrad colors
#define AMS_COLOR_BLACK     0x54
#define AMS_COLOR_MARINE    0x44
#define AMS_COLOR_BLUE      0x55
#define AMS_COLOR_DARK_RED  0x5C
#define AMS_COLOR_PURPLE    0x58
#define AMS_COLOR_VIOLET    0x5D
#define AMS_COLOR_RED       0x4C
#define AMS_COLOR_BARBIE    0x45
#define AMS_COLOR_MAGENTA   0x4D
#define AMS_COLOR_DGREEN    0x56
#define AMS_COLOR_SARCELLE  0x46
#define AMS_COLOR_DODGER    0x57
#define AMS_COLOR_OLIVE     0x5E
#define AMS_COLOR_GREY      0x40
#define AMS_COLOR_LILA      0x5F
#define AMS_COLOR_ORANGE    0x4E
#define AMS_COLOR_PINK      0x47
#define AMS_COLOR_BONBON    0x4F
#define AMS_COLOR_LIME      0x52
#define AMS_COLOR_LEAF      0x42
#define AMS_COLOR_CYAN      0x53
#define AMS_COLOR_CHARTR    0x5A
#define AMS_COLOR_LGREEN    0x59
#define AMS_COLOR_SKY       0x5B
#define AMS_COLOR_YELLOW    0x4A
#define AMS_COLOR_LYELLOW   0x43
#define AMS_COLOR_WHITE     0x4B
