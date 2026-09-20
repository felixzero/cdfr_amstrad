#include <string.h>

#include "graphics.h"
#include "sprites.h"

extern uint8_t robot_1[];

extern uint8_t block_1e[], block_2e[], block_3e[], block_3s[];

main()
{
    set_palette(0, AMS_COLOR_MAGENTA);
    set_palette(1, AMS_COLOR_BLACK);
    set_palette(2, AMS_COLOR_MARINE);
    set_palette(3, AMS_COLOR_BLUE);
    set_palette(4, AMS_COLOR_DARK_RED);
    set_palette(5, AMS_COLOR_RED);
    set_palette(6, AMS_COLOR_DGREEN);
    set_palette(7, AMS_COLOR_DODGER);
    set_palette(8, AMS_COLOR_OLIVE);
    set_palette(9, AMS_COLOR_GREY);
    set_palette(10, AMS_COLOR_ORANGE);
    set_palette(11, AMS_COLOR_PINK);
    set_palette(12, AMS_COLOR_SKY);
    set_palette(13, AMS_COLOR_YELLOW);
    set_palette(14, AMS_COLOR_LYELLOW);
    set_palette(15, AMS_COLOR_WHITE);


    // Copy background to video ram
    memcpy((void*)0xC000, (void*)0x4000, 0x4000);
    swap_buffers();

    struct rect r = {
        .x = 0,
        .y = 0,
        .w = 12,
        .h = 26
    };
    sprite_handle_t robot_sprite = create_sprite(robot_1, &r);

    struct rect r2 = {
        .x = 90,
        .y = 44,
        .w = 14,
        .h = 23
    };
    create_sprite(block_3e, &r2);

    struct rect r3 = {
        .x = 116,
        .y = 74,
        .w = 14,
        .h = 19
    };
    create_sprite(block_2e, &r3);

    struct rect r4 = {
        .x = 138,
        .y = 100,
        .w = 14,
        .h = 15
    };
    create_sprite(block_1e, &r4);

    struct rect r5 = {
        .x = 50,
        .y = 65,
        .w = 14,
        .h = 23
    };
    create_sprite(block_3s, &r5);

    initialize_all_sprites();

    struct point s = {
        .x = 0,
        .y = 0,
    };

    while (1) {
        s.x++;
        s.y++;
        if (s.x > 150) {
            s.x = 0;
            s.y = 0;
        }
        move_sprite(robot_sprite, &s);

        update_sprites();
        wait_for_vsync();
        swap_buffers();
    }
}
