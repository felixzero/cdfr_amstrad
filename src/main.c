#include <string.h>

#include "graphics.h"
#include "sprites.h"

extern uint8_t element_jeu_bidon[];
extern uint8_t robot[];

main()
{
    // Copy background to video ram
    memcpy((void*)0xC000, (void*)0x4000, 0x4000);
    swap_buffers();

    struct rect r;

    r.x = 10;
    r.y = 80;
    r.w = 16;
    r.h = 32;

    sprite_handle_t robot_sprite = create_sprite(robot, &r);

    sprite_handle_t bidon_sprites[16];
    r.w = 8;
    r.h = 8;
    r.y = 10;
    for (uint8_t i = 0; i < 16; ++i) {
        r.x = 8 * i;
        bidon_sprites[i] = create_sprite(element_jeu_bidon, &r);
    }

    initialize_all_sprites();

    struct point p = {
        .x = 10,
        .y = 80,
    };
    struct point q = {
        .x = 0,
        .y = 10,
    };
    uint8_t current_moving_bidon = 0;
    while (1) {
        p.x += 2;
        if (p.x >= 150) {
            p.x = 10;
        }
        move_sprite(robot_sprite, &p);

        q.y += 2;
        if (q.y >= 190) {
            q.y = 10;
            current_moving_bidon++;
            current_moving_bidon %= 16;
            q.x = current_moving_bidon * 8;
        }
        move_sprite(bidon_sprites[current_moving_bidon], &q);

        update_sprites();

        wait_for_vsync();
        swap_buffers();
    }
}
