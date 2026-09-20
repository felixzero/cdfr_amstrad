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
    uint8_t z_index;
    struct point past;
    struct point current;
    struct point future;
    const uint8_t *graphics_to_update;
    uint8_t need_redraw;
};

#define NUMBER_OF_SCREEN_BUFFERS 2

#define MAX_NUMBER_OF_SPRITES 32
struct sprite_info sprite_info_table[MAX_NUMBER_OF_SPRITES];
uint8_t sprite_requested_drawing_order[MAX_NUMBER_OF_SPRITES];
uint8_t sprite_effective_drawing_order[NUMBER_OF_SCREEN_BUFFERS * MAX_NUMBER_OF_SPRITES];
static uint8_t number_of_sprites = 0;

#define BUFFER_HEAP_SIZE 4*1024
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
    sprite_info_table[number_of_sprites].z_index = 127;
    sprite_info_table[number_of_sprites].past.x = dimensions->x;
    sprite_info_table[number_of_sprites].past.y = dimensions->y;
    sprite_info_table[number_of_sprites].current.x = dimensions->x;
    sprite_info_table[number_of_sprites].current.y = dimensions->y;
    sprite_info_table[number_of_sprites].future.x = dimensions->x;
    sprite_info_table[number_of_sprites].future.y = dimensions->y;
    sprite_info_table[number_of_sprites].graphics_to_update = NULL;
    sprite_info_table[number_of_sprites].need_redraw = 0;

    number_of_sprites++;
    return number_of_sprites - 1;
}

void move_sprite(sprite_handle_t sprite, struct point *position)
{
    sprite_info_table[sprite].future.x = position->x;
    sprite_info_table[sprite].future.y = position->y;
    sprite_info_table[sprite].need_redraw = NUMBER_OF_SCREEN_BUFFERS;
}

void set_sprite_z_index(sprite_handle_t sprite, uint8_t z_index)
{
    sprite_info_table[sprite].z_index = z_index;
    sprite_info_table[sprite].need_redraw = NUMBER_OF_SCREEN_BUFFERS;
}

void change_sprite_asset(sprite_handle_t sprite, const uint8_t *graphics)
{
    sprite_info_table[sprite].graphics_to_update = graphics;

    uint16_t sprite_buffer_size = sprite_info_table[sprite].w * sprite_info_table[sprite].h / 2;
    uint8_t current_buffer = get_current_buffer();

    memcpy(*(&sprite_info_table[sprite].buffer_1 + current_buffer), graphics, sprite_buffer_size);

    sprite_info_table[sprite].need_redraw = NUMBER_OF_SCREEN_BUFFERS;
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
            sprite_effective_drawing_order[MAX_NUMBER_OF_SPRITES * buffer + i] = i;
        }
        swap_buffers();
    }

    recalculate_drawing_order();
}

void clear_sprites(void)
{
    static struct rect r;
    uint8_t current_buffer = get_current_buffer();

    for (sprite_handle_t i = 0; i < number_of_sprites; ++i) {
        sprite_handle_t j = sprite_effective_drawing_order[MAX_NUMBER_OF_SPRITES * current_buffer + i];
        
        if (sprite_info_table[i].need_redraw) {
            r.w = sprite_info_table[j].w;
            r.h = sprite_info_table[j].h;
            r.x = sprite_info_table[j].past.x;
            r.y = sprite_info_table[j].past.y;
            blit_sprite_swap(*(&sprite_info_table[j].buffer_1 + current_buffer), &r);
        }

        if (sprite_info_table[j].graphics_to_update) {
            uint16_t sprite_buffer_size = sprite_info_table[j].w * sprite_info_table[j].h / 2;
            memcpy(
                *(&sprite_info_table[j].buffer_1 + current_buffer),
                sprite_info_table[j].graphics_to_update,
                sprite_buffer_size
            );
            sprite_info_table[number_of_sprites].graphics_to_update = NULL;
        }
    }
}

void draw_sprites(void)
{
    static struct rect r;
    uint8_t current_buffer = get_current_buffer();

    for (sprite_handle_t i = number_of_sprites - 1; i != 0xFF; --i) {
        if (sprite_info_table[i].need_redraw) {
            sprite_handle_t j = sprite_requested_drawing_order[i];
            sprite_effective_drawing_order[MAX_NUMBER_OF_SPRITES * current_buffer + i] = j;

            r.w = sprite_info_table[j].w;
            r.h = sprite_info_table[j].h;
            r.x = sprite_info_table[j].future.x;
            r.y = sprite_info_table[j].future.y;
            blit_sprite_swap(*(&sprite_info_table[j].buffer_1 + current_buffer), &r);

            sprite_info_table[j].past = sprite_info_table[j].current;
            sprite_info_table[j].current = sprite_info_table[j].future;

            sprite_info_table[i].need_redraw--;
        }
    }
}

void recalculate_drawing_order(void)
{
    for (uint8_t i = 0; i < number_of_sprites; ++i) {
        sprite_requested_drawing_order[i] = i;
    }

    for (uint8_t i = 0; i < number_of_sprites; ++i) {
        uint8_t x = sprite_requested_drawing_order[i];
        uint8_t j = i;
        while (j > 0 && sprite_info_table[sprite_requested_drawing_order[j - 1]].z_index > sprite_info_table[x].z_index) {
            sprite_requested_drawing_order[j] = sprite_requested_drawing_order[j - 1];
            j--;
        }
        sprite_requested_drawing_order[j] = x;

        sprite_info_table[i].need_redraw = 2;
    }
}
