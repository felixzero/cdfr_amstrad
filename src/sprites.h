#pragma once

#include "graphics.h"

typedef uint8_t sprite_handle_t;

// Register a new sprite, setting graphics, size and position
sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *rect);

// Change sprite position
void move_sprite(sprite_handle_t sprite, struct point *position);

// Update sprite z_index
// If z_index == 0, the sprite is not displayed
void set_sprite_z_index(sprite_handle_t sprite, uint8_t z_index);

// Flag the sprite for redraw, as well as other sprites intersecting it
void trigger_sprite_redraw(sprite_handle_t sprite);

// Change the graphics of a sprite, to another one of the same dimensions
void change_sprite_asset(sprite_handle_t sprite, const uint8_t *graphics);

// Update display of sprites
void draw_sprites(void);
