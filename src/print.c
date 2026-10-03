#include "print.h"

#include "graphics.h"

extern const uint8_t font_glyphs[];

static uint8_t cursor_x, cursor_y;
static uint8_t foreground_color, background_color;

#define GLYPH_WIDTH 6
#define GLYPH_HEIGHT 7
#define COLOR_SCRAMBLE(x) (((x) & (1 << 0)) << 6 | ((x) & (1 << 1)) << 1 | ((x) & (1 << 2)) << 2 | ((x) & (1 << 3)) >> 3)

#define VIDEO_RAM_BLOCK 0x0800

void move_cursor(uint8_t col, uint8_t line)
{
    cursor_x = col;
    cursor_y = line;
}

void set_text_palette(uint8_t foreground, uint8_t background)
{
    foreground_color = COLOR_SCRAMBLE(foreground);
    background_color = COLOR_SCRAMBLE(background);
}

void prints(const char *s)
{
    while (*s) {
        printc(*(s++));
    }
}

void printc(char c)
{
    static uint8_t *screen_addr;
    static const uint8_t *glyph;
    static uint8_t i, j;
    static uint8_t glyph_line;

    screen_addr = VIDEO_RAM_START + 80 * cursor_y + cursor_x * (GLYPH_WIDTH / 2);

    if ((c >= 'A') && (c <= 'Z')) {
        glyph = &font_glyphs[(c - 'A') * GLYPH_HEIGHT];
    } else if ((c >= 'a') && (c <= 'z')) {
        glyph = &font_glyphs[(c - 'a' + 'Z' - 'A' + 1) * GLYPH_HEIGHT];
    } else if ((c >= '0') && (c <= '9')) {
        glyph = &font_glyphs[(c - '0' + 'Z' - 'A' + 'z' - 'a' + 2) * GLYPH_HEIGHT];
    } else {
        for (i = 0; i < GLYPH_HEIGHT; ++i) {
            for (j = 0; j < GLYPH_WIDTH / 2; ++j) {
                screen_addr[j] = background_color | (background_color << 1);
            }
            screen_addr += VIDEO_RAM_BLOCK;
        }

        cursor_x++;
        return;
    }

    for (i = 0; i < GLYPH_HEIGHT; ++i) {
        glyph_line = glyph[i];

        for (j = 0; j < GLYPH_WIDTH; ++j) {
            if (j % 2 == 0) {
                screen_addr[j / 2] = 0;
                screen_addr[j / 2] |= (glyph_line & (1 << j)) ? foreground_color << 1 : background_color << 1;
            } else {
                screen_addr[j / 2] |= (glyph_line & (1 << j)) ? foreground_color << 0 : background_color << 0;
            }
        }
        screen_addr += VIDEO_RAM_BLOCK;
    }

    cursor_x++;
}

void printint(uint8_t v)
{
    static char buffer[4];

    buffer[0] = (v / 100) + '0';
    buffer[1] = ((v / 10) % 10) + '0';
    buffer[2] = (v % 10) + '0';
    buffer[3] = 0;

    prints(buffer);
}
