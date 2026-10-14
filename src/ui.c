#include "ui.h"
#include "graphics.h"
#include "sprite_assets.h"

#define DIGIT_LED_LINES         7
#define DIGIT_LINE_SPACING      5

// First nibble codes the state of the two leftmost LED ; second nibble the other two
static const uint8_t digits[10][DIGIT_LED_LINES] = {
    { 0x33, 0x21, 0x21, 0x21, 0x21, 0x21, 0x33 }, // 0
    { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 }, // 1
    { 0x33, 0x01, 0x01, 0x33, 0x20, 0x20, 0x33 }, // 2
    { 0x33, 0x01, 0x01, 0x33, 0x01, 0x01, 0x33 }, // 3
    { 0x21, 0x21, 0x21, 0x33, 0x01, 0x01, 0x01 }, // 4
    { 0x33, 0x20, 0x20, 0x33, 0x01, 0x01, 0x33 }, // 5
    { 0x33, 0x20, 0x20, 0x33, 0x21, 0x21, 0x33 }, // 6
    { 0x33, 0x01, 0x01, 0x13, 0x01, 0x01, 0x01 }, // 7
    { 0x33, 0x21, 0x21, 0x33, 0x21, 0x21, 0x33 }, // 8
    { 0x33, 0x21, 0x21, 0x33, 0x01, 0x01, 0x33 }, // 9
};

static const uint8_t *digit_asset_lookup_table[] = {
    asset_digit_00,
    asset_digit_01,
    asset_digit_10,
    asset_digit_11,
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

#define INITIAL_VALUE   99
static int8_t printed_clock_digits[NUMBER_OF_BUFFERS * NUMBER_OF_CLOCK_DIGITS] = {
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
    asset_word_mine,
    asset_word_build,
    asset_word_pick,
    asset_word_put,
    asset_word_ready,
    asset_word_fire
};

static uint8_t printed_user_messages[NUMBER_OF_BUFFERS * NUMBER_OF_PLAYERS] = {
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
};

uint8_t user_messages[NUMBER_OF_PLAYERS] = {
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_NONE,
};

static uint8_t *question_lookup[QUESTION_LENGTH] = {
    0,
    asset_question_ready,
    asset_question_will_start,
};

static uint8_t printed_question[NUMBER_OF_BUFFERS] = {
    QUESTION_NONE,
    QUESTION_NONE,
};

uint8_t question = QUESTION_NONE;

static const struct rect brick_ui_rects[] = {
    {
        .x = 2,
        .y = 36,
        .w = ASSET_BRICK_UI_WIDTH,
        .h = ASSET_BRICK_UI_HEIGHT
    },
    {
        .x = 10,
        .y = 36,
        .w = ASSET_BRICK_UI_WIDTH,
        .h = ASSET_BRICK_UI_HEIGHT
    },
    {
        .x = 18,
        .y = 36,
        .w = ASSET_BRICK_UI_WIDTH,
        .h = ASSET_BRICK_UI_HEIGHT
    },
    {
        .x = 134,
        .y = 188,
        .w = ASSET_BRICK_UI_WIDTH,
        .h = ASSET_BRICK_UI_HEIGHT
    },
    {
        .x = 142,
        .y = 188,
        .w = ASSET_BRICK_UI_WIDTH,
        .h = ASSET_BRICK_UI_HEIGHT
    },
    {
        .x = 150,
        .y = 188,
        .w = ASSET_BRICK_UI_WIDTH,
        .h = ASSET_BRICK_UI_HEIGHT
    },
};

uint8_t user_bricks[NUMBER_OF_PLAYERS] = { 0, 0 };
static uint8_t printed_user_bricks[NUMBER_OF_BUFFERS * NUMBER_OF_PLAYERS] = { 0, 0, 0, 0 };

static uint8_t current_buffer, i;

static void clear_ui_elements_for_buffer(void);
static void blit_digits(uint8_t digit_1, uint8_t digit_2);

void init_ui(void)
{
    blit_digits(clock_digits[0], clock_digits[1]);
    swap_buffers();
    blit_digits(clock_digits[0], clock_digits[1]);
    swap_buffers();
}


bool decrement_game_clock(void)
{
    clock_digits[1]--;
    if (clock_digits[1] < 0) {
        clock_digits[1] = 9;
        clock_digits[0]--;
    }
    if ((clock_digits[0] == 0) && (clock_digits[1] == 0)) {
        return true;
    }

    return false;
}


void update_ui_display(void)
{
    static uint8_t *current_printed_user_message, *current_printed_question, *current_printed_user_bricks;

    current_buffer = get_current_buffer();

    if (printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 1] != clock_digits[1]) {
        blit_digits(
            printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 0],
            printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 1]
        );
        printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 0] = clock_digits[0];
        printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 1] = clock_digits[1];
        blit_digits(
            clock_digits[0],
            clock_digits[1]
        );
    }

    for (i = 0; i < NUMBER_OF_PLAYERS; ++i) {
        current_printed_user_message = &printed_user_messages[NUMBER_OF_PLAYERS * current_buffer + i];

        if (*current_printed_user_message != user_messages[i]) {
            if (*current_printed_user_message) {
                blit_sprite_xor(user_message_lookup[*current_printed_user_message], players_action_rect + i);
            }
            *current_printed_user_message = user_messages[i];
            if (user_messages[i]) {
                blit_sprite_xor(user_message_lookup[user_messages[i]], players_action_rect + i);
            }
        }

        current_printed_user_bricks = &printed_user_bricks[NUMBER_OF_PLAYERS * current_buffer + i];
        while (*current_printed_user_bricks > user_bricks[i]) {
            (*current_printed_user_bricks)--;
            blit_sprite_xor(asset_brick_ui, &brick_ui_rects[i * MAX_NUMBER_OF_BRICKS + *current_printed_user_bricks]);
        }
        while (*current_printed_user_bricks < user_bricks[i]) {
            blit_sprite_xor(asset_brick_ui, &brick_ui_rects[i * MAX_NUMBER_OF_BRICKS + *current_printed_user_bricks]);
            (*current_printed_user_bricks)++;
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
    current_buffer = get_current_buffer();
    
    clear_ui_elements_for_buffer();
    swap_buffers();
    current_buffer = 1 - current_buffer;
    clear_ui_elements_for_buffer();
    swap_buffers();
}


static void clear_ui_elements_for_buffer(void)
{
    static uint8_t *current_printed_user_message, *current_printed_question, current_printed_user_bricks;

    current_printed_question = &printed_question[current_buffer];

    blit_digits(
        printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 0],
        printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 1]
    );
    for (i = 0; i < NUMBER_OF_PLAYERS; ++i) {
        current_printed_user_message = &printed_user_messages[NUMBER_OF_PLAYERS * current_buffer + i];
        if (*current_printed_user_message != PLAYER_MESSAGE_NONE) {
            blit_sprite_xor(user_message_lookup[*current_printed_user_message], players_action_rect + i);
        }
        
        current_printed_user_bricks = printed_user_bricks[NUMBER_OF_PLAYERS * current_buffer + i];
        while (current_printed_user_bricks > 0) {
            current_printed_user_bricks--;
            blit_sprite_xor(asset_brick_ui, &brick_ui_rects[i * MAX_NUMBER_OF_BRICKS + current_printed_user_bricks]);
        }
    }
    if (*current_printed_question) {
        blit_sprite_xor(question_lookup[*current_printed_question], &question_rect);
    }
}

static void blit_digits(uint8_t digit_1, uint8_t digit_2)
{
    static struct rect r;
    static const uint8_t *digit_shape;

    r.w = ASSET_DIGIT_00_WIDTH;
    r.h = ASSET_DIGIT_00_HEIGHT;

    r.x = 126;
    r.y = 12;
    digit_shape = &digits[digit_1][0];
    for (uint8_t i = 0; i < DIGIT_LED_LINES; ++i) {
        blit_sprite_xor(digit_asset_lookup_table[*digit_shape >> 4], &r);
        r.y += DIGIT_LINE_SPACING;
        digit_shape++;
    }

    r.x = 132;
    r.y = 18;
    digit_shape = &digits[digit_1][0];
    for (uint8_t i = 0; i < DIGIT_LED_LINES; ++i) {
        blit_sprite_xor(digit_asset_lookup_table[*digit_shape & 0x0F], &r);
        r.y += DIGIT_LINE_SPACING;
        digit_shape++;
    }

    r.x = 140;
    r.y = 26;
    digit_shape = &digits[digit_2][0];
    for (uint8_t i = 0; i < DIGIT_LED_LINES; ++i) {
        blit_sprite_xor(digit_asset_lookup_table[*digit_shape >> 4], &r);
        r.y += DIGIT_LINE_SPACING;
        digit_shape++;
    }

    r.x = 146;
    r.y = 32;
    digit_shape = &digits[digit_2][0];
    for (uint8_t i = 0; i < DIGIT_LED_LINES; ++i) {
        blit_sprite_xor(digit_asset_lookup_table[*digit_shape & 0x0F], &r);
        r.y += DIGIT_LINE_SPACING;
        digit_shape++;
    }
}
