#include "ui.h"
#include "graphics.h"
#include "sprite_assets.h"

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

static const struct rect players_action_rect[] = {
    {
        .x = 34,
        .y = 5,
        .w = ASSET_WORD_MINE_WIDTH,
        .h = ASSET_WORD_MINE_HEIGHT
    },
    {
        .x = 136,
        .y = 169,
        .w = ASSET_WORD_MINE_WIDTH,
        .h = ASSET_WORD_MINE_HEIGHT
    }
};

static const struct rect question_rect = {
    .x = 14,
    .y = 170,
    .w = ASSET_QUESTION_READY_WIDTH,
    .h = ASSET_QUESTION_READY_HEIGHT
};

#define INITIAL_VALUE   88
static const uint8_t *digit_assets[10];

static int8_t printed_clock_digits[NUMBER_OF_BUFFERS * 2];
int8_t clock_digits[2];

static uint8_t *user_message_lookup[PLAYER_MESSAGE_LENGTH];
static uint8_t printed_user_messages[NUMBER_OF_BUFFERS * 2];
uint8_t user_messages[2];

static uint8_t *question_lookup[QUESTION_LENGTH];
static uint8_t printed_question[NUMBER_OF_BUFFERS];
uint8_t question;

void init_ui(void)
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

    user_message_lookup[PLAYER_MESSAGE_NONE] = 0;
    user_message_lookup[PLAYER_MESSAGE_READY] = asset_word_ready;
    user_message_lookup[PLAYER_MESSAGE_MINE] = asset_word_mine;
    user_message_lookup[PLAYER_MESSAGE_BUILD] = asset_word_build;

    for (i = 0; i < NUMBER_OF_BUFFERS; ++i) {
        printed_clock_digits[2 * i + 0] = INITIAL_VALUE / 10;
        printed_clock_digits[2 * i + 1] = INITIAL_VALUE % 10;
    }
    clock_digits[0] = INITIAL_VALUE / 10;
    clock_digits[1] = INITIAL_VALUE % 10;

    for (i = 0; i < 2; ++i) {
        user_messages[i] = PLAYER_MESSAGE_NONE;
        printed_user_messages[i] = PLAYER_MESSAGE_NONE;
        printed_user_messages[2 + i] = PLAYER_MESSAGE_NONE;
    }

    question_lookup[QUESTION_NONE] = 0;
    question_lookup[QUESTION_READY] = asset_question_ready;
    question_lookup[QUESTION_WILL_START] = asset_question_will_start;
    question_lookup[QUESTION_FINISHED] = asset_question_finished;

    question = QUESTION_NONE;
    printed_question[0] = QUESTION_NONE;
    printed_question[1] = QUESTION_NONE;

    blit_sprite_xor(digit_assets[clock_digits[0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[clock_digits[1]], &right_digit_rect);
    swap_buffers();
    blit_sprite_xor(digit_assets[clock_digits[0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[clock_digits[1]], &right_digit_rect);
    swap_buffers();
}


bool decrement_game_clock(void)
{
    clock_digits[1]--;
    if (clock_digits[1] < 0) {
        clock_digits[1] = 9;
        clock_digits[0]--;
    }
    if (clock_digits[0] < 0) {
        clock_digits[0] = 9;
        clock_digits[1] = 9;
        return true;
    }

    return false;
}


void update_ui_display(void)
{
    static uint8_t current_buffer, i, *current_printed_user_message, *current_printed_question;

    current_buffer = get_current_buffer();

    if (printed_clock_digits[2 * current_buffer + 1] != clock_digits[1]) {
        blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 0]], &left_digit_rect);
        blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 1]], &right_digit_rect);
        printed_clock_digits[2 * current_buffer + 0] = clock_digits[0];
        printed_clock_digits[2 * current_buffer + 1] = clock_digits[1];
        blit_sprite_xor(digit_assets[clock_digits[0]], &left_digit_rect);
        blit_sprite_xor(digit_assets[clock_digits[1]], &right_digit_rect);
    }

    for (i = 0; i < 2; ++i) {
        current_printed_user_message = &printed_user_messages[2 * current_buffer + i];

        if (*current_printed_user_message != user_messages[i]) {
            if (*current_printed_user_message) {
                blit_sprite_xor(user_message_lookup[*current_printed_user_message], players_action_rect + i);
            }
            *current_printed_user_message = user_messages[i];
            if (user_messages[i]) {
                blit_sprite_xor(user_message_lookup[user_messages[i]], players_action_rect + i);
            }
        }
    }

    current_printed_question = &printed_question[current_buffer];
    if (*current_printed_question != question) {
        if (*current_printed_question) {
            blit_sprite_xor(question_lookup[*current_printed_question], &question_rect);
        }
        *current_printed_question = question;
        if (question) {
            blit_sprite_xor(question_lookup[question], &question_rect);
        }
    }
}
