#include "model.h"
#include "controller.h"
#include "inputs.h"

#include <string.h>

struct robot_model robots[2] = {
    { .position = {
        .x = 0,
        .y = ROBOT_STARTING_POSITION_V,
        .w = 2 * ROBOT_HALF_WIDTH,
        .h = 2 * ROBOT_HALF_WIDTH
    }, .orientation = ORIENTATION_EAST },
    { .position = {
        .x = TABLE_EDGE_U - 2 * ROBOT_HALF_WIDTH,
        .y = ROBOT_STARTING_POSITION_V,
        .w = 2 * ROBOT_HALF_WIDTH,
        .h = 2 * ROBOT_HALF_WIDTH
    }, .orientation = ORIENTATION_WEST },
};

int8_t grail_locations[2] = { GRAIL_HOLDERS_ID_START, GRAIL_HOLDERS_ID_START + 1 };

struct rect obstacles[NUMBER_OF_OBSTACLES] = {
    // W-E oriented quarries
    { .x = 6, .y = 1, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = 28, .y = 1, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = 6, .y = TABLE_EDGE_V - 1 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = 28, .y = TABLE_EDGE_V - 1 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = TABLE_EDGE_U - 6 - BLOCK_WIDTH, .y = 1, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = TABLE_EDGE_U - 28 - BLOCK_WIDTH, .y = 1, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = TABLE_EDGE_U - 6 - BLOCK_WIDTH, .y = TABLE_EDGE_V - 1 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = TABLE_EDGE_U - 28 - BLOCK_WIDTH, .y = TABLE_EDGE_V - 1 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    // N-S oriented quarries
    { .x = TABLE_EDGE_U / 2 - BLOCK_WIDTH / 2, .y = 16, .w = BLOCK_WIDTH, .h = BLOCK_LENGTH },
    { .x = TABLE_EDGE_U / 2 - BLOCK_WIDTH / 2, .y = TABLE_EDGE_V - 16 - BLOCK_LENGTH, .w = BLOCK_WIDTH, .h = BLOCK_LENGTH },
    // W-E player 1 walls
    { .x = 4, .y = 14, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = 20, .y = 14, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = 4, .y = TABLE_EDGE_V - 14 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = 20, .y = TABLE_EDGE_V - 14 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    // Player 1 gate
    { .x = 24, .y = TABLE_EDGE_V / 2 - BLOCK_LENGTH / 2, .w = BLOCK_WIDTH, .h = BLOCK_LENGTH },
    // W-E player 2 walls
    { .x = TABLE_EDGE_U - 4 - BLOCK_LENGTH, .y = 14, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = TABLE_EDGE_U - 20 - BLOCK_LENGTH, .y = 14, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = TABLE_EDGE_U - 4 - BLOCK_LENGTH, .y = TABLE_EDGE_V - 14 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    { .x = TABLE_EDGE_U - 20 - BLOCK_LENGTH, .y = TABLE_EDGE_V - 14 - BLOCK_WIDTH, .w = BLOCK_LENGTH, .h = BLOCK_WIDTH },
    // Player 2 gate
    { .x = TABLE_EDGE_U - 25 - BLOCK_WIDTH, .y = TABLE_EDGE_V / 2 - BLOCK_LENGTH / 2, .w = BLOCK_WIDTH, .h = BLOCK_LENGTH },
    // Player 1 towers
    { .x = 12, .y = 14, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    { .x = 12, .y = TABLE_EDGE_V - 14 - TOWER_WIDTH, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    { .x = 24, .y = 20, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    { .x = 24, .y = TABLE_EDGE_V - 20 - TOWER_WIDTH, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    // Player 2 towers
    { .x = TABLE_EDGE_U - 12 - TOWER_WIDTH, .y = 14, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    { .x = TABLE_EDGE_U - 12 - TOWER_WIDTH, .y = TABLE_EDGE_V - 14 - TOWER_WIDTH, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    { .x = TABLE_EDGE_U - 24 - TOWER_WIDTH, .y = 20, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    { .x = TABLE_EDGE_U - 24 - TOWER_WIDTH, .y = TABLE_EDGE_V - 20 - TOWER_WIDTH, .w = TOWER_WIDTH, .h = TOWER_WIDTH },
    // Player 1 grail holder
    { .x = 0, .y = 22, .w = GRAIL_WIDTH, .h = GRAIL_WIDTH },
    // Player 2 grail holder
    { .x = TABLE_EDGE_U - GRAIL_WIDTH, .y = 42, .w = GRAIL_WIDTH, .h = GRAIL_WIDTH },
    // Player 1 PAMI
    { .x = 0, .y = 8, .w = PAMI_WIDTH, .h = PAMI_WIDTH },
    // Player 2 PAMI
    { .x = TABLE_EDGE_U - PAMI_WIDTH, .y = 52, .w = PAMI_WIDTH, .h = PAMI_WIDTH },
};

uint8_t obstacle_stone_quantity[NUMBER_OF_OBSTACLES] = {
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    INITIAL_NUMBER_OF_STONES,
    0,
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

struct ball_model balls[] = {
    // Player 1 balls
    { .flags = BALL_FLAGS_PLAYER_ID_0 },
    { .flags = BALL_FLAGS_PLAYER_ID_0 },
    { .flags = BALL_FLAGS_PLAYER_ID_0 },
    // Player 2 balls
    { .flags = BALL_FLAGS_PLAYER_ID_1 },
    { .flags = BALL_FLAGS_PLAYER_ID_1 },
    { .flags = BALL_FLAGS_PLAYER_ID_1 },
};

// [int(40 * (1 - (i - 7.5) ** 2 / 8 ** 2)) for i in range(16)]
const uint8_t ball_parabola[BALL_PARABOLA_LENGTH] = { 4, 13, 21, 27, 32, 36, 38, 39, 39, 38, 36, 32, 27, 21, 13, 4 };

uint8_t launched_balls[NUMBER_OF_PLAYERS] = { 0, 0 };
