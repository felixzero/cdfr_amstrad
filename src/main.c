#include <string.h>

#include "graphics.h"

extern uint8_t element_jeu_bidon[];
extern uint8_t robot[];

struct rect r;
struct rect r_robot;

main()
{
    // Copy background to video ram
    memcpy((void*)0xC000, (void*)0x4000, 0x4000);
    swap_buffers();

    r_robot.x = 0;
    r_robot.y = 80;
    r_robot.w = 16;
    r_robot.h = 32;

    r.x = 0;
    r.y = 10;
    r.w = 8;
    r.h = 8;

    r.y = 10;
    for (uint8_t x = 0; x < 152; x += 8) {
        r.x = x;
        blit_sprite_xor(element_jeu_bidon, &r);
    }
    blit_sprite_xor(robot, &r_robot);
    swap_buffers();
    r.y = 20;
    for (uint8_t x = 0; x < 152; x += 8) {
        r.x = x;
        blit_sprite_xor(element_jeu_bidon, &r);
    }
    r_robot.x = 2;
    blit_sprite_xor(robot, &r_robot);
    swap_buffers();
    r_robot.x = 4;

    r.y = 30;

    while (1) {
        if (r.y == 10) {
            r.y = 170;
            for (uint8_t x = 0; x < 152; x += 8) {
                r.x = x;
                blit_sprite_xor(element_jeu_bidon, &r);
            }
            r.y = 10;
        } else if (r.y == 20) {
            r.y = 180;
            for (uint8_t x = 0; x < 152; x += 8) {
                r.x = x;
                blit_sprite_xor(element_jeu_bidon, &r);
            }
            r.y = 20;
        } else {
            r.y -= 20;
            for (uint8_t x = 0; x < 152; x += 8) {
                r.x = x;
                blit_sprite_xor(element_jeu_bidon, &r);
            }
            r.y += 20;
        }

        if (r_robot.x == 0) {
            r_robot.x = 116;
            blit_sprite_xor(robot, &r_robot);
            r_robot.x = 0;
        } else if (r_robot.x == 2) {
            r_robot.x = 118;
            blit_sprite_xor(robot, &r_robot);
            r_robot.x = 2;
        } else {
            r_robot.x -= 4;
            blit_sprite_xor(robot, &r_robot);
            r_robot.x += 4;
        }

        for (uint8_t x = 0; x < 152; x += 8) {
            r.x = x;
            blit_sprite_xor(element_jeu_bidon, &r);
        }

        blit_sprite_xor(robot, &r_robot);

        wait_for_vsync();
        swap_buffers();

        r.y += 10;
        if (r.y == 190) {
            r.y = 10;
        }


        r_robot.x += 2;
        if (r_robot.x >= 120) {
            r_robot.x = 0;
        }
    }


    while(1) {};
}
