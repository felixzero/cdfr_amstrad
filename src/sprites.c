#define NUMBER_OF_BUFFERS 2
#define MAX_NUMBER_OF_SPRITES 32

#include "sprites.h"
#include "sprite_assets.h"
#include <string.h>

#define STATUS_NO_ERASE         0
#define STATUS_DISPLAYED_FLAG   (1 << 0)
#define STATUS_NEED_REDRAW_FLAG (1 << 1)
#define STATUS_GRAPHICS_CHANGED (1 << 2)

struct blitted_sprite {
    struct rect rect;
    const uint8_t *screen_backup;
    uint8_t status;
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
    uint8_t z_index;
};

static struct screen_history history[NUMBER_OF_BUFFERS];
static struct requested_sprite requested_sprites[MAX_NUMBER_OF_SPRITES];
static sprite_handle_t requested_blit_order[MAX_NUMBER_OF_SPRITES];
static uint8_t number_of_sprites = 0;

uint8_t buffer_heap[ASSET_TOTAL_SIZE];
static uint16_t allocated_size = 0;

sprite_handle_t create_sprite(const uint8_t *graphics, struct rect *rect)
{
    static uint16_t sprite_buffer_size;
    static uint8_t i;
    
    sprite_buffer_size = rect->w * rect->h / 2;

    requested_sprites[number_of_sprites].graphics = graphics;
    memcpy(&requested_sprites[number_of_sprites].rect, rect, sizeof(struct rect));
    requested_sprites[number_of_sprites].z_index = 0;

    for (i = 0; i < NUMBER_OF_BUFFERS; ++i) {
        memset(&history[i].blitted_sprites[number_of_sprites], 0, sizeof(struct blitted_sprite));
        history[i].blitted_sprites[number_of_sprites].status |= STATUS_GRAPHICS_CHANGED;
        history[i].blitted_sprites[number_of_sprites].screen_backup = buffer_heap + allocated_size;
        history[i].blitted_sprites[number_of_sprites].rect.w = rect->w;
        history[i].blitted_sprites[number_of_sprites].rect.h = rect->h;
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
    static struct rect r;
    static uint8_t buffer;
    static sprite_handle_t i, effective;

    for (buffer = 0; buffer < NUMBER_OF_BUFFERS; ++buffer) {
        r.x = requested_sprites[sprite].rect.x;
        r.y = requested_sprites[sprite].rect.y;
        r.w = requested_sprites[sprite].rect.w;
        r.h = requested_sprites[sprite].rect.h;

        rect_merge_into(&r, &history[buffer].blitted_sprites[sprite].rect);

        for (i = 0; i < number_of_sprites; ++i) {
            effective = history[buffer].blit_order[i];
            if (rect_intersects(&r, &history[buffer].blitted_sprites[effective].rect)) {
                history[buffer].blitted_sprites[effective].status |= STATUS_NEED_REDRAW_FLAG;
                rect_merge_into(&r, &history[buffer].blitted_sprites[effective].rect);
            }
        }
    }
}


void draw_sprites(void)
{
    static uint8_t current_buffer;
    static sprite_handle_t i, j, effective;
    static struct blitted_sprite* blit;
    static struct requested_sprite *request;

    current_buffer = get_current_buffer();

    // First undo the elements that need to be redrawn, in reverse blitting order
    for (i = number_of_sprites - 1; i != 0xFF; --i) {
        effective = history[current_buffer].blit_order[i];
        blit = &history[current_buffer].blitted_sprites[effective];
        request = &requested_sprites[effective];

        if ((blit->status & STATUS_NEED_REDRAW_FLAG) && (blit->status & STATUS_DISPLAYED_FLAG)) {
            blit_sprite_swap(blit->screen_backup, &blit->rect);
        }
    }

    for (i = 1; i < number_of_sprites; ++i) {
        effective = requested_blit_order[i];
        j = i;
        while (j > 0 && requested_sprites[requested_blit_order[j - 1]].z_index < requested_sprites[effective].z_index) {
            requested_blit_order[j] = requested_blit_order[j - 1];
            j--;
        }
        requested_blit_order[j] = effective;
    }

    // Then we draw the updated elements
    for (i = 0; i < number_of_sprites; ++i) {
        effective = requested_blit_order[i];
        blit = &history[current_buffer].blitted_sprites[effective];
        request = &requested_sprites[effective];
        
        // Do not show sprites with zero z-index
        if ((blit->status & STATUS_NEED_REDRAW_FLAG) && request->z_index) {
            if (blit->status & STATUS_GRAPHICS_CHANGED) {
                memcpy(blit->screen_backup, request->graphics, (request->rect.w * request->rect.h) / 2);
            }

            blit_sprite_swap(blit->screen_backup, &request->rect);
            memcpy(&blit->rect, &request->rect, sizeof(struct rect));

            blit->status = STATUS_DISPLAYED_FLAG;
        }

        history[current_buffer].blit_order[i] = effective;
    }
}
