#pragma once

#include "graphics.h"

typedef uint8_t sprite_handle_t;

sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *dimensions);

void move_sprite(sprite_handle_t sprite, struct point *position);

void initialize_all_sprites(void);

void update_sprites(void);
