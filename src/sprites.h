#pragma once

#include "graphics.h"

#include <stdbool.h>

typedef uint8_t sprite_handle_t;

void init_sprites(void);

// Register a new sprite, setting graphics, size and position
sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *rect);

// Change sprite position
void move_sprite(sprite_handle_t sprite, struct point *position);

// Update sprite z_index
// If z_index < 0, the sprite is not displayed
void set_sprite_z_index(sprite_handle_t sprite, int8_t z_index);

// Hide or display the sprite
void set_sprite_visibility(sprite_handle_t sprite, bool visible);

// Flag the sprite for redraw, as well as other sprites intersecting it
void trigger_sprite_redraw(sprite_handle_t sprite);

// Change the graphics of a sprite, to another one of the same dimensions
void change_sprite_asset(sprite_handle_t sprite, const uint8_t *graphics);

// Update display of sprites
void draw_sprites(void);

#define MAX_NUMBER_OF_SPRITES 40

struct blitted_sprite {
    struct rect rect;
    const uint8_t *screen_backup;
    uint8_t status;
    uint8_t __padding;
};

struct screen_history
{
    sprite_handle_t blit_order[MAX_NUMBER_OF_SPRITES];
    struct blitted_sprite blitted_sprites[MAX_NUMBER_OF_SPRITES];
};

struct requested_sprite
{
    const uint8_t *graphics;
    struct rect rect;
    int8_t z_index;
    uint8_t size;
};


