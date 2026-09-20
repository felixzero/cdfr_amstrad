#include "sprites.h"

#include <string.h>

// Past is whatever was on the previous occurence of the current frame
// Current is whatever is on the last alternate frame
// Future is what needs to be drawn on the current frame
struct sprite_info {
    const uint8_t *buffer_1;
    const uint8_t *buffer_2;
    uint8_t w;
    uint8_t h;
    struct point past;
    struct point current;
    struct point future;
};

#define NUMBER_OF_SCREEN_BUFFERS 2

#define MAX_NUMBER_OF_SPRITES 64
struct sprite_info sprite_info_table[MAX_NUMBER_OF_SPRITES];
static uint8_t number_of_sprites = 0;

#define BUFFER_HEAP_SIZE 2048
uint8_t buffer_heap[BUFFER_HEAP_SIZE];
static uint16_t allocated_size = 0;


sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *dimensions)
{
    uint16_t sprite_buffer_size = dimensions->w * dimensions->h / 2;

    sprite_info_table[number_of_sprites].buffer_1 = buffer_heap + allocated_size;
    allocated_size += sprite_buffer_size;
    sprite_info_table[number_of_sprites].buffer_2 = buffer_heap + allocated_size;
    allocated_size += sprite_buffer_size;

    memcpy(sprite_info_table[number_of_sprites].buffer_1, graphics, sprite_buffer_size);
    memcpy(sprite_info_table[number_of_sprites].buffer_2, graphics, sprite_buffer_size);

    sprite_info_table[number_of_sprites].w = dimensions->w;
    sprite_info_table[number_of_sprites].h = dimensions->h;
    sprite_info_table[number_of_sprites].past.x = dimensions->x;
    sprite_info_table[number_of_sprites].past.y = dimensions->y;
    sprite_info_table[number_of_sprites].current.x = dimensions->x;
    sprite_info_table[number_of_sprites].current.y = dimensions->y;
    sprite_info_table[number_of_sprites].future.x = dimensions->x;
    sprite_info_table[number_of_sprites].future.y = dimensions->y;

    return number_of_sprites++;
}

void move_sprite(sprite_handle_t sprite, struct point *position)
{
    sprite_info_table[sprite].future.x = position->x;
    sprite_info_table[sprite].future.y = position->y;
}

void initialize_all_sprites(void)
{
    static struct rect r;

    for (uint8_t buffer = 0; buffer < NUMBER_OF_SCREEN_BUFFERS; ++buffer) {
        for (sprite_handle_t i = 0; i < number_of_sprites; ++i) {
            // Draw them in their initial position
            r.w = sprite_info_table[i].w;
            r.h = sprite_info_table[i].h;
            r.x = sprite_info_table[i].current.x;
            r.y = sprite_info_table[i].current.y;
            blit_sprite_swap(*(&sprite_info_table[i].buffer_1 + buffer), &r);
        }
        swap_buffers();
    }
}

void update_sprites(void)
{
    static struct rect r;
    uint8_t current_buffer = get_current_buffer();

    // Undo old sprites
    for (sprite_handle_t i = 0; i < number_of_sprites; ++i) {
        r.w = sprite_info_table[i].w;
        r.h = sprite_info_table[i].h;
        r.x = sprite_info_table[i].past.x;
        r.y = sprite_info_table[i].past.y;
        blit_sprite_swap(*(&sprite_info_table[i].buffer_1 + current_buffer), &r);
    }

    // Draw new sprites
    for (sprite_handle_t i = number_of_sprites - 1; i != 0xFF; --i) {
        r.w = sprite_info_table[i].w;
        r.h = sprite_info_table[i].h;
        r.x = sprite_info_table[i].future.x;
        r.y = sprite_info_table[i].future.y;
        blit_sprite_swap(*(&sprite_info_table[i].buffer_1 + current_buffer), &r);

        sprite_info_table[i].past = sprite_info_table[i].current;
        sprite_info_table[i].current = sprite_info_table[i].future;
    }
}
