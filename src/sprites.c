#include "sprites.h"

// Past is whatever was on the previous occurence of the current frame
// Current is whatever is on the last alternate frame
// Future is what needs to be drawn on the current frame
struct sprite_info {
    const uint8_t *graphics;
    uint8_t w;
    uint8_t h;
    struct point past;
    struct point current;
    struct point future;
    uint8_t is_moving;
};

#define MAX_NUMBER_OF_SPRITES 64

struct sprite_info sprite_info_table[MAX_NUMBER_OF_SPRITES];
static uint8_t number_of_sprites = 0;

sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *dimensions)
{
    sprite_info_table[number_of_sprites].graphics = graphics;
    sprite_info_table[number_of_sprites].w = dimensions->w;
    sprite_info_table[number_of_sprites].h = dimensions->h;
    sprite_info_table[number_of_sprites].past.x = dimensions->x;
    sprite_info_table[number_of_sprites].past.y = dimensions->y;
    sprite_info_table[number_of_sprites].current.x = dimensions->x;
    sprite_info_table[number_of_sprites].current.y = dimensions->y;
    sprite_info_table[number_of_sprites].future.x = dimensions->x;
    sprite_info_table[number_of_sprites].future.y = dimensions->y;
    sprite_info_table[number_of_sprites].is_moving = 0;

    return number_of_sprites++;
}

void move_sprite(sprite_handle_t sprite, struct point *position)
{
    sprite_info_table[sprite].future.x = position->x;
    sprite_info_table[sprite].future.y = position->y;

    sprite_info_table[sprite].is_moving = 1;
}

void initialize_all_sprites(void)
{
    static struct rect r;

    for (uint8_t repetition = 0; repetition < 2; ++repetition) {
        for (sprite_handle_t i = 0; i < number_of_sprites; ++i) {
            // Draw them in their initial position
            r.w = sprite_info_table[i].w;
            r.h = sprite_info_table[i].h;
            r.x = sprite_info_table[i].current.x;
            r.y = sprite_info_table[i].current.y;
            blit_sprite_xor(sprite_info_table[i].graphics, &r);
            swap_buffers();
        }
    }
}

void update_sprites(void)
{
    static struct rect r;

    for (sprite_handle_t i = 0; i < number_of_sprites; ++i) {
        if (!sprite_info_table[i].is_moving) {
            continue;
        }

        // Erase the past
        r.w = sprite_info_table[i].w;
        r.h = sprite_info_table[i].h;
        r.x = sprite_info_table[i].past.x;
        r.y = sprite_info_table[i].past.y;
        blit_sprite_xor(sprite_info_table[i].graphics, &r);

        // Draw the future
        r.x = sprite_info_table[i].future.x;
        r.y = sprite_info_table[i].future.y;
        blit_sprite_xor(sprite_info_table[i].graphics, &r);

        // Set the correct information for the next frame
        sprite_info_table[i].past = sprite_info_table[i].current;
        sprite_info_table[i].current = sprite_info_table[i].future;

        if (
            (sprite_info_table[i].past.x == sprite_info_table[i].future.x)
            && (sprite_info_table[i].past.y == sprite_info_table[i].future.y)
        ) {
            sprite_info_table[i].is_moving = 0;
        }
    }
}
