#include "model_view.h"

#include "game_model.h"
#include "sprites.h"
#include "inputs.h"

static sprite_handle_t robot_sprite;
struct point s = {
    .x = 0,
    .y = 0,
};

#define STONE_SPRITE_W 14
#define STONE_SPRITE_H 23
static const uint8_t quarry_x_positions[] = { 68, 90, 10, 32, 138, 116, 80, 58, 86, 60 };
static const uint8_t quarry_y_positions[] = { 22, 44, 80, 102, 92, 70, 150, 128, 73, 100 };
static sprite_handle_t quarry_sprites[NUMBER_OF_QUARRIES];

extern uint8_t robot_1s[], robot_1n[];

extern uint8_t block_1e[], block_2e[], block_3e[], block_3s[];

void init_model_view(void)
{
    struct rect r = {
        .x = 0,
        .y = 0,
        .w = 12,
        .h = 26
    };
    robot_sprite = create_sprite(robot_1s, &r);

    r.w = STONE_SPRITE_W;
    r.h = STONE_SPRITE_H;
    for (uint8_t i = 0; i < NUMBER_OF_QUARRIES - 2; ++i) {
        r.x = quarry_x_positions[i];
        r.y = quarry_y_positions[i];
        quarry_sprites[i] = create_sprite(block_3e, &r);
        set_sprite_z_index(quarry_sprites[i], 255 - r.y);
    }
    for (uint8_t i = NUMBER_OF_QUARRIES - 2; i < NUMBER_OF_QUARRIES; ++i) {
        r.x = quarry_x_positions[i];
        r.y = quarry_y_positions[i];
        quarry_sprites[i] = create_sprite(block_3s, &r);
        set_sprite_z_index(quarry_sprites[i], 255 - r.y);
    }

    initialize_all_sprites();
    //set_sprite_z_index(robot_sprite, 200);
}

void update_graphics(void)
{
    uint32_t keys = get_keypress();
    if (!(keys & KEY_D)) {
        s.x += 2;
        if (s.x > 150) {
            s.x = 150;
        }
    }
    if (!(keys & KEY_Q)) {
        s.x -= 2;
        if (s.x == 0xFE) {
            s.x = 0;
        }
    }
    if (!(keys & KEY_S)) {
        s.y += 2;
        if (s.y > 160) {
            s.y = 160;
        }
    }
    if (!(keys & KEY_Z)) {
        s.y -= 2;
        if (s.y == 0xFE) {
            s.y = 0;
        }
        
    }
    
    move_sprite(robot_sprite, &s);
    set_sprite_z_index(robot_sprite, 255 - s.y);

    clear_sprites();
    recalculate_drawing_order();
    /*if (!(keys & KEY_Z)) {
        change_sprite_asset(robot_sprite, robot_1n);
    }*/
    draw_sprites();
    wait_for_vsync();
    swap_buffers();
}