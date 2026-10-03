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
static const uint8_t *digit_assets[10] = {
    asset_digit_0,
    asset_digit_1,
    asset_digit_2,
    asset_digit_3,
    asset_digit_4,
    asset_digit_5,
    asset_digit_6,
    asset_digit_7,
    asset_digit_8,
    asset_digit_9,
};

static int8_t printed_clock_digits[NUMBER_OF_BUFFERS * 2] = {
    INITIAL_VALUE / 10,
    INITIAL_VALUE % 10,
    INITIAL_VALUE / 10,
    INITIAL_VALUE % 10,
};

int8_t clock_digits[2] = {
    INITIAL_VALUE / 10,
    INITIAL_VALUE % 10,
};

static uint8_t *user_message_lookup[PLAYER_MESSAGE_LENGTH] = {
    0,
    asset_word_ready,
    asset_word_mine,
    asset_word_build,
};

static uint8_t printed_user_messages[NUMBER_OF_BUFFERS * 2] = {
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
};

uint8_t user_messages[2] = {
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
};

static uint8_t *question_lookup[QUESTION_LENGTH] = {
    0,
    asset_question_ready,
    asset_question_will_start,
    asset_question_finished,
};

static uint8_t printed_question[NUMBER_OF_BUFFERS] = {
    QUESTION_NONE,
    QUESTION_NONE,
};

uint8_t question = QUESTION_NONE;

void init_ui(void)
{
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

void clear_ui_elements(void)
{
    static uint8_t current_buffer, i, *current_printed_user_message, *current_printed_question;

    current_buffer = get_current_buffer();
    current_printed_question = &printed_question[current_buffer];

    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 1]], &right_digit_rect);
    for (i = 0; i < 2; ++i) {
        current_printed_user_message = &printed_user_messages[2 * current_buffer + i];
        if (*current_printed_user_message != PLAYER_MESSAGE_NONE) {
            blit_sprite_xor(user_message_lookup[*current_printed_user_message], players_action_rect + i);
        }
    }
    if (*current_printed_question) {
        blit_sprite_xor(question_lookup[*current_printed_question], &question_rect);
    }

    swap_buffers();
    current_buffer = 1 - current_buffer;

    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 0]], &left_digit_rect);
    blit_sprite_xor(digit_assets[printed_clock_digits[2 * current_buffer + 1]], &right_digit_rect);
    for (i = 0; i < 2; ++i) {
        current_printed_user_message = &printed_user_messages[2 * current_buffer + i];
        if (*current_printed_user_message != PLAYER_MESSAGE_NONE) {
            blit_sprite_xor(user_message_lookup[*current_printed_user_message], players_action_rect + i);
        }
    }
    if (*current_printed_question) {
        blit_sprite_xor(question_lookup[*current_printed_question], &question_rect);
    }

    swap_buffers();
}
