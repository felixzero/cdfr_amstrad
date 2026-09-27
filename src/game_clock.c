#include "game_clock.h"
#include "graphics.h"
#include "sprite_assets.h"

#define INITIAL_VALUE   99

static const uint8_t *digit_assets[10];

static const struct rect left_digit_rect = {
    .x = 126,
    .y = 12,
    .w = ASSET_DIGIT_0_WIDTH,
    .h = ASSET_DIGIT_0_HEIGHT
};

static const struct rect right_digit_rect = {
    .x = 140,
    .y = 26,
    .w = ASSET_DIGIT_0_WIDTH,
    .h = ASSET_DIGIT_0_HEIGHT
};

static int8_t printed_clock_digits[NUMBER_OF_BUFFERS * 2];
static int8_t current_clock_digits[2];

void init_game_clock(void)
{
    uint8_t i;

    digit_assets[0] = asset_digit_0;
    digit_assets[1] = asset_digit_1;
    digit_assets[2] = asset_digit_2;
    digit_assets[3] = asset_digit_3;
    digit_assets[4] = asset_digit_4;
    digit_assets[5] = asset_digit_5;
    digit_assets[6] = asset_digit_6;
    digit_assets[7] = asset_digit_7;
    digit_assets[8] = asset_digit_8;
    digit_assets[9] = asset_digit_9;

    for (i = 0; i < NUMBER_OF_BUFFERS; ++i) {
        printed_clock_digits[2 * i + 0] = INITIAL_VALUE / 10;
        printed_clock_digits[2 * i + 1] = INITIAL_VALUE % 10;
    }
    current_clock_digits[0] = INITIAL_VALUE / 10;
    current_clock_digits[1] = INITIAL_VALUE % 10;

    blit_sprite_xor(digit_assets[current_clock_digits[0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[current_clock_digits[1]], &right_digit_rect);
    swap_buffers();
    blit_sprite_xor(digit_assets[current_clock_digits[0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[current_clock_digits[1]], &right_digit_rect);
    swap_buffers();
}

void decrement_game_clock(void)
{
    current_clock_digits[1]--;
    if (current_clock_digits[1] < 0) {
        current_clock_digits[1] = 9;
        current_clock_digits[0]--;
    }
    if (current_clock_digits[0] < 0) {
        current_clock_digits[0] = 9;
        current_clock_digits[1] = 9;
    }
}

void update_clock_display(void)
{
    static uint8_t current_buffer;

    current_buffer = get_current_buffer();

    if (printed_clock_digits[2 * current_buffer + 1] == current_clock_digits[1]) {
        return;
    }

    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 1]], &right_digit_rect);

    printed_clock_digits[2 * current_buffer + 0] = current_clock_digits[0];
    printed_clock_digits[2 * current_buffer + 1] = current_clock_digits[1];

    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 1]], &right_digit_rect);
}

