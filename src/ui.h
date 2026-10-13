#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "model.h"

#define NUMBER_OF_CLOCK_DIGITS  2

// First elements have to coincide with MINING_INTERACTIONS
enum {
    PLAYER_MESSAGE_NONE,
    PLAYER_MESSAGE_MINE,
    PLAYER_MESSAGE_BUILD,
    PLAYER_MESSAGE_PICK,
    PLAYER_MESSAGE_PUT,
    PLAYER_MESSAGE_READY,
    PLAYER_MESSAGE_FIRE,
    PLAYER_MESSAGE_LENGTH
};

enum {
    QUESTION_NONE,
    QUESTION_READY,
    QUESTION_WILL_START,
    QUESTION_LENGTH
};

void init_ui(void);
bool decrement_game_clock(void);
void update_ui_display(void);
void clear_ui_elements(void);

extern uint8_t user_messages[NUMBER_OF_PLAYERS];
extern uint8_t question;
extern int8_t clock_digits[NUMBER_OF_PLAYERS];
extern uint8_t user_bricks[NUMBER_OF_PLAYERS];
