#pragma once

#include "graphics.h"

typedef uint8_t sprite_handle_t;

sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *dimensions);

void move_sprite(sprite_handle_t sprite, struct point *position);

void set_sprite_z_index(sprite_handle_t sprite, uint8_t z_index);

void change_sprite_asset(sprite_handle_t sprite, const uint8_t *graphics);

void initialize_all_sprites(void);

void clear_sprites(void);

void draw_sprites(void);

void recalculate_drawing_order(void);
