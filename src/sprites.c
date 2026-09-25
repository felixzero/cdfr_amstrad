#define NUMBER_OF_BUFFERS 2
#define MAX_NUMBER_OF_SPRITES 32

#include "sprites.h"
#include <string.h>

#define STATUS_NO_ERASE         0
#define STATUS_DISPLAYED_FLAG   (1 << 0)
#define STATUS_NEED_REDRAW_FLAG (1 << 1)
#define STATUS_GRAPHICS_CHANGED (1 << 2)

struct blitted_sprite {
    struct point position;
    const uint8_t *screen_backup;
    uint8_t status;
};

struct screen_history
{
    struct blitted_sprite blitted_sprites[MAX_NUMBER_OF_SPRITES];
    sprite_handle_t blit_order[MAX_NUMBER_OF_SPRITES];
};

struct requested_sprite
{
    const uint8_t *graphics;
    struct rect rect;
    uint8_t z_index;
};

static struct screen_history history[NUMBER_OF_BUFFERS];
static struct requested_sprite requested_sprites[MAX_NUMBER_OF_SPRITES];
static sprite_handle_t requested_blit_order[MAX_NUMBER_OF_SPRITES];
static uint8_t number_of_sprites = 0;

#define BUFFER_HEAP_SIZE 4*1024
uint8_t buffer_heap[BUFFER_HEAP_SIZE];
static uint16_t allocated_size = 0;

sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *rect)
{
    uint16_t sprite_buffer_size = rect->w * rect->h / 2;

    requested_sprites[number_of_sprites].graphics = graphics;
    memcpy(&requested_sprites[number_of_sprites].rect, rect, sizeof(struct rect));
    requested_sprites[number_of_sprites].z_index = 0;

    for (uint8_t i = 0; i < NUMBER_OF_BUFFERS; ++i) {
        memset(&history[i].blitted_sprites[number_of_sprites], 0, sizeof(struct blitted_sprite));
        history[i].blitted_sprites[number_of_sprites].status |= STATUS_GRAPHICS_CHANGED;
        history[i].blitted_sprites[number_of_sprites].screen_backup = buffer_heap + allocated_size;
        allocated_size += sprite_buffer_size;
        history[i].blit_order[number_of_sprites] = number_of_sprites;
    }

    requested_blit_order[number_of_sprites] = number_of_sprites;
    return number_of_sprites++;
}

void move_sprite(sprite_handle_t sprite, struct point *position)
{
    requested_sprites[sprite].rect.x = position->x;
    requested_sprites[sprite].rect.y = position->y;
}

void set_sprite_z_index(sprite_handle_t sprite, uint8_t z_index)
{
    requested_sprites[sprite].z_index = z_index;
}

void change_sprite_asset(sprite_handle_t sprite, const uint8_t *graphics)
{
    requested_sprites[sprite].graphics = graphics;
    history[0].blitted_sprites[sprite].status |= STATUS_GRAPHICS_CHANGED;
    history[1].blitted_sprites[sprite].status |= STATUS_GRAPHICS_CHANGED;
}

void trigger_sprite_redraw(sprite_handle_t sprite)
{
    for (uint8_t buffer = 0; buffer < NUMBER_OF_BUFFERS; ++buffer) {
        struct rect r = {
            .x = requested_sprites[sprite].rect.x,
            .y = requested_sprites[sprite].rect.y,
            .w = requested_sprites[sprite].rect.w,
            .h = requested_sprites[sprite].rect.h,
        };
        struct rect r_blit = {
            .x = history[buffer].blitted_sprites[sprite].position.x,
            .y = history[buffer].blitted_sprites[sprite].position.y,
            .w = r.w,
            .h = r.h,
        };
        rect_merge_into(&r, &r_blit);

        for (sprite_handle_t i = 0; i < number_of_sprites; ++i) {
            sprite_handle_t effective = history[buffer].blit_order[i];
            r_blit.x = history[buffer].blitted_sprites[effective].position.x;
            r_blit.y = history[buffer].blitted_sprites[effective].position.y;
            r_blit.w = requested_sprites[effective].rect.w;
            r_blit.h = requested_sprites[effective].rect.h;
            if (rect_intersects(&r, &r_blit)) {
                history[buffer].blitted_sprites[effective].status |= STATUS_NEED_REDRAW_FLAG;
                rect_merge_into(&r, &r_blit);
            }
        }
    }
}


void draw_sprites(void)
{
    static struct rect r;
    uint8_t current_buffer = get_current_buffer();

    // First undo the elements that need to be redrawn, in reverse blitting order
    for (sprite_handle_t i = number_of_sprites - 1; i != 0xFF; --i) {
        sprite_handle_t effective = history[current_buffer].blit_order[i];
        struct blitted_sprite* blit = &history[current_buffer].blitted_sprites[effective];
        struct requested_sprite *request = &requested_sprites[effective];

        if ((blit->status & STATUS_NEED_REDRAW_FLAG) && (blit->status & STATUS_DISPLAYED_FLAG)) {
            r.x = blit->position.x;
            r.y = blit->position.y;
            r.w = request->rect.w;
            r.h = request->rect.h;
            blit_sprite_swap(blit->screen_backup, &r);
        }
    }

    for (sprite_handle_t i = 1; i < number_of_sprites; ++i) {
        uint8_t x = requested_blit_order[i];
        uint8_t j = i;
        while (j > 0 && requested_sprites[requested_blit_order[j - 1]].z_index < requested_sprites[x].z_index) {
            requested_blit_order[j] = requested_blit_order[j - 1];
            j--;
        }
        requested_blit_order[j] = x;
    }

    // Then we draw the updated elements
    for (sprite_handle_t i = 0; i < number_of_sprites; ++i) {
        sprite_handle_t effective = requested_blit_order[i];
        struct blitted_sprite *blit = &history[current_buffer].blitted_sprites[effective];
        struct requested_sprite *request = &requested_sprites[effective];
        
        // Do not show sprites with zero z-index
        if ((blit->status & STATUS_NEED_REDRAW_FLAG) && request->z_index) {
            r.x = request->rect.x;
            r.y = request->rect.y;
            r.w = request->rect.w;
            r.h = request->rect.h;

            if (blit->status & STATUS_GRAPHICS_CHANGED) {
                memcpy(blit->screen_backup, request->graphics, (r.w * r.h) / 2);
            }

            blit_sprite_swap(blit->screen_backup, &r);
            blit->position.x = r.x;
            blit->position.y = r.y;
            blit->status = STATUS_DISPLAYED_FLAG;
        }

        history[current_buffer].blit_order[i] = effective;
    }
}
