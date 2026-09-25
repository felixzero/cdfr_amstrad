#pragma once

#include "graphics.h"

typedef uint8_t sprite_handle_t;

sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *rect);

void move_sprite(sprite_handle_t sprite, struct point *position);

void set_sprite_z_index(sprite_handle_t sprite, uint8_t z_index);

void trigger_sprite_redraw(sprite_handle_t sprite);

void change_sprite_asset(sprite_handle_t sprite, const uint8_t *graphics);

void draw_sprites(void);
