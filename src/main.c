#include <string.h>

#include "graphics.h"
#include "inputs.h"
#include "controller.h"
#include "print.h"
#include "sprite_assets.h"
#include "audio.h"

#define GAME_FPS_DESAMPLING     3

void display_title_screen(void);

void main()
{
__asm
    ; Stack pointer at top of usable memory
    ld sp, #(0xBFFF - FB_BUFFER_HEAP_SIZE)

    ; Set mode 0 and black border
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

    display_title_screen();

    init_controller();

    while (1) {
        update_controller();

        wait_for_vsync(GAME_FPS_DESAMPLING);
        swap_buffers();
    }
}

void display_title_screen(void)
{
    static uint8_t *screen, i;
    static struct rect r;

    init_audio_player(TITLE_SCREEN_START);
    set_double_buffering(false);

    memset(VIDEO_RAM_START, 0xC0, 0x4000);
    set_text_palette(15, 1);
    move_cursor(3, 15);
    prints("The legend of Camelot");
    move_cursor(6, 17);
    prints("Coupe de France");
    move_cursor(7, 18);
    prints("de robotique");
    move_cursor(4, 20);
    prints("Un jeu non officiel");
    move_cursor(6, 21);
    prints("par les ESCROCS");
    move_cursor(20, 22);
    prints("2026");
    move_cursor(4, 23);
    prints("Appuyez sur A");
    move_cursor(4, 24);
    prints("pour commencer");

    screen = VIDEO_RAM_START;
    while (screen >= VIDEO_RAM_START) {

        for (int i = 0; i < 25; ++i) {
            *(screen + 80 * i) = 0xF3;
            *(screen + 80 * i + 2) = 0xF3;
            *(screen + 80 * i + 4) = 0xF3;
            *(screen + 80 * i + 79) = 0xF3;
            *(screen + 80 * i + 77) = 0xF3;
            *(screen + 80 * i + 75) = 0xF3;
        }
        screen += 0x0800;
    }

    r.w = ASSET_LOGO_WIDTH;
    r.h = ASSET_LOGO_HEIGHT;
    r.x = 57;
    r.y = 40;
    blit_sprite_xor(asset_logo, &r);

    while (1) {
        wait_for_vsync(0);
        get_keypress();
        if (is_key_pressed(0, check_key_action)) {
            set_double_buffering(true);
            stop_audio_player();
            return;
        }
        play_audio_player();
    }
}
