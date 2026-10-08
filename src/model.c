#include "model.h"
#include "controller.h"
#include "inputs.h"

#include <string.h>

uint8_t obstacle_stone_quantity[NUMBER_OF_OBSTACLES] = { 0 };

struct robot_model robots[2] = {
    { .position = { .x = 0, .y = ROBOT_STARTING_POSITION_V }, .orientation = ORIENTATION_EAST },
    { .position = { .x = TABLE_EDGE_U, .y = ROBOT_STARTING_POSITION_V }, .orientation = ORIENTATION_WEST },
};

int8_t grail_locations[2] = { GRAIL_HOLDERS_ID_START, GRAIL_HOLDERS_ID_START + 1 };

struct rect obstacles[NUMBER_OF_OBSTACLES] = {
    // W-E oriented quarries
    { .x = 4, .y = (uint8_t)-5, .w = 14, .h = 10 },
    { .x = 26, .y = (uint8_t)-5, .w = 14, .h = 10 },
    { .x = 4, .y = 53, .w = 14, .h = 10 },
    { .x = 26, .y = 53, .w = 14, .h = 10 },
    { .x = 74, .y = (uint8_t)-5, .w = 14, .h = 10 },
    { .x = 52, .y = (uint8_t)-5, .w = 14, .h = 10 },
    { .x = 74, .y = 53, .w = 14, .h = 10 },
    { .x = 52, .y = 53, .w = 14, .h = 10 },
    // N-S oriented quarries
    { .x = 38, .y = 11, .w = 10, .h = 14 },
    { .x = 39, .y = 38, .w = 10, .h = 14 },
    // W-E player 1 walls
    { .x = (uint8_t)-3, .y = 8, .w = 14, .h = 10 },
    { .x = 13, .y = 8, .w = 14, .h = 10 },
    { .x = (uint8_t)-4, .y = 41, .w = 14, .h = 10 },
    { .x = 12, .y = 41, .w = 14, .h = 10 },
    // Player 1 gate
    { .x = 16, .y = 25, .w = 10, .h = 14 },
    // W-E player 2 walls
    { .x = 80, .y = 7, .w = 14, .h = 10 },
    { .x = 65, .y = 8, .w = 14, .h = 10 },
    { .x = 80, .y = 41, .w = 14, .h = 10 },
    { .x = 66, .y = 41, .w = 14, .h = 10 },
    // Player 2 gate
    { .x = 61, .y = 24, .w = 10, .h = 14 },
    // Player 1 towers
    { .x = 9, .y = 10, .w = 6, .h = 6 },
    { .x = 8, .y = 43, .w = 6, .h = 6 },
    { .x = 21, .y = 18, .w = 6, .h = 6 },
    { .x = 21, .y = 36, .w = 6, .h = 6 },
    // Player 2 towers
    { .x = 73, .y = 10, .w = 6, .h = 6 },
    { .x = 76, .y = 43, .w = 6, .h = 6 },
    { .x = 64, .y = 15, .w = 6, .h = 6 },
    { .x = 65, .y = 36, .w = 6, .h = 6 },
    // Player 1 grail holder
    { .x = (uint8_t)-4, .y = 17, .w = 6, .h = 6 },
    // Player 2 grail holder
    { .x = 87, .y = 17, .w = 6, .h = 6 },
    // Player 1 PAMI
    { .x = (uint8_t)-4, .y = 6, .w = 6, .h = 6 },
    // Player 2 PAMI
    { .x = 88, .y = 50, .w = 6, .h = 6 },
};

const uint8_t obstacle_flags[NUMBER_OF_OBSTACLES] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x82,
    0x04, 0x04, 0x04, 0x04, 0x84,
    0x05, 0x05, 0x05, 0x05, 0x85,
    0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09,
    0x0C, 0x0D,
    0x10, 0x11,
};

bool pami_stuck[2] = {
    false, false,
};

