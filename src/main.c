#include <string.h>

#include "graphics.h"
#include "inputs.h"
#include "model_view.h"

main()
{
    // Set mode 0 and black border
__asm
    ld bc, #0x7F8C
    out (c), c

    ld bc, #0x7F14
    out (c), c
    ld bc, #0x7f54
    out (c), c
__endasm;

    // Configure palette
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

    init_model_view();

    while (1) {
        update_graphics();
    }
}
