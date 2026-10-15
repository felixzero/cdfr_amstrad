#include "ui.h"
#include "graphics.h"
#include "sprite_assets.h"

#define DIGIT_LED_LINES         7
#define DIGIT_LINE_SPACING      5

#define DESTINATION_FIRST_DIGIT { digit_blit_destination.x = 126; digit_blit_destination.y = 12; }
#define DESTINATION_SECOND_DIGIT { digit_blit_destination.x = 140; digit_blit_destination.y = 26; }

// First nibble codes the state of the two leftmost LED ; second nibble the other two
static const uint8_t digits[10][DIGIT_LED_LINES + 1] = {
    { 0x33, 0x21, 0x21, 0x21, 0x21, 0x21, 0x33, 0 }, // 0
    { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0 }, // 1
    { 0x33, 0x01, 0x01, 0x33, 0x20, 0x20, 0x33, 0 }, // 2
    { 0x33, 0x01, 0x01, 0x33, 0x01, 0x01, 0x33, 0 }, // 3
    { 0x21, 0x21, 0x21, 0x33, 0x01, 0x01, 0x01, 0 }, // 4
    { 0x33, 0x20, 0x20, 0x33, 0x01, 0x01, 0x33, 0 }, // 5
    { 0x33, 0x20, 0x20, 0x33, 0x21, 0x21, 0x33, 0 }, // 6
    { 0x33, 0x01, 0x01, 0x13, 0x01, 0x01, 0x01, 0 }, // 7
    { 0x33, 0x21, 0x21, 0x33, 0x21, 0x21, 0x33, 0 }, // 8
    { 0x33, 0x21, 0x21, 0x33, 0x01, 0x01, 0x33, 0 }, // 9
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

#define INITIAL_VALUE   20
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

static struct rect digit_blit_destination = {
    .w = ASSET_DIGIT_00_WIDTH,
    .h = ASSET_DIGIT_00_HEIGHT
};

static void clear_ui_elements_for_buffer(void);
static void blit_digit(int8_t previous_digit, int8_t digit);

void init_ui(void)
{
    DESTINATION_FIRST_DIGIT;
    blit_digit(-1, clock_digits[0]);
    DESTINATION_SECOND_DIGIT;
    blit_digit(-1, clock_digits[1]);
    swap_buffers();
    DESTINATION_FIRST_DIGIT;
    blit_digit(-1, clock_digits[0]);
    DESTINATION_SECOND_DIGIT;
    blit_digit(-1, clock_digits[1]);
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
        DESTINATION_FIRST_DIGIT;
        blit_digit(
            printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 0],
            clock_digits[0]
        );
        printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 0] = clock_digits[0];

        DESTINATION_SECOND_DIGIT;
        blit_digit(
            printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 1],
            clock_digits[1]
        );
        printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 1] = clock_digits[1];
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

    DESTINATION_FIRST_DIGIT;
    blit_digit(printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 0], -1);
    DESTINATION_SECOND_DIGIT;
    blit_digit(printed_clock_digits[NUMBER_OF_CLOCK_DIGITS * current_buffer + 1], -1);

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

static void blit_digit(int8_t previous_digit, int8_t digit)
{
    static const uint8_t *digit_shape, *previous_digit_shape;

    if (digit >= 0) {
        digit_shape = &digits[digit][0];
    } else {
        digit_shape = 0;
    }
    if (previous_digit >= 0) {
        previous_digit_shape = &digits[previous_digit][0];
    } else {
        previous_digit_shape = 0;
    }

    for (uint8_t i = 0; i < DIGIT_LED_LINES; ++i) {
        // Remove old element if needed
        if (previous_digit_shape && (!digit_shape || ((*digit_shape >> 4) != (*previous_digit_shape >> 4)))) {
            blit_sprite_xor(digit_asset_lookup_table[*previous_digit_shape >> 4], &digit_blit_destination);
        }
        digit_blit_destination.x += 6;
        digit_blit_destination.y += 6;
        if (previous_digit_shape && (!digit_shape || ((*digit_shape & 0x0F) != (*previous_digit_shape & 0x0F)))) {
            blit_sprite_xor(digit_asset_lookup_table[*previous_digit_shape & 0x0F], &digit_blit_destination);
        }
        digit_blit_destination.x -= 6;
        digit_blit_destination.y -= 6;

        // Draw new element
        if (digit_shape && (!previous_digit_shape || ((*digit_shape >> 4) != (*previous_digit_shape >> 4)))) {
            blit_sprite_xor(digit_asset_lookup_table[*digit_shape >> 4], &digit_blit_destination);
        }
        digit_blit_destination.x += 6;
        digit_blit_destination.y += 6;
        if (digit_shape && (!previous_digit_shape || ((*digit_shape & 0x0F) != (*previous_digit_shape & 0x0F)))) {
            blit_sprite_xor(digit_asset_lookup_table[*digit_shape & 0x0F], &digit_blit_destination);
        }
        digit_blit_destination.x -= 6;
        digit_blit_destination.y -= 6;

        digit_blit_destination.y += DIGIT_LINE_SPACING;
        if (digit_shape) {
            digit_shape++;
        }
        if(previous_digit_shape) {
            previous_digit_shape++;
        }
    }
}
