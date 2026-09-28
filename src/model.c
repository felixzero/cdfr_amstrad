#include "model.h"
#include "controller.h"
#include "inputs.h"

#include <string.h>

const struct rect obstacles[] = {
    // W-E oriented quarries
    { .x =  4, .y = -5, .w = 14, .h = 10 },
    { .x = 26, .y = -5, .w = 14, .h = 10 },
    { .x =  4, .y = 53, .w = 14, .h = 10 },
    { .x = 26, .y = 53, .w = 14, .h = 10 },
    { .x = 74, .y = -5, .w = 14, .h = 10 },
    { .x = 52, .y = -5, .w = 14, .h = 10 },
    { .x = 74, .y = 53, .w = 14, .h = 10 },
    { .x = 52, .y = 53, .w = 14, .h = 10 },
    // N-S oriented quarries
    { .x = 38, .y = 11, .w = 10, .h = 14 },
    { .x = 39, .y = 38, .w = 10, .h = 14 },
    // W-E player 1 walls
    { .x = -3, .y =  8, .w = 14, .h = 10 },
    { .x = 13, .y =  8, .w = 14, .h = 10 },
    { .x = -4, .y = 41, .w = 14, .h = 10 },
    { .x = 12, .y = 41, .w = 14, .h = 10 },
    // Player 1 gate
    { .x = 16, .y = 25, .w = 10, .h = 14 },
    // W-E player 2 walls
    { .x = 80, .y =  7, .w = 14, .h = 10 },
    { .x = 65, .y =  8, .w = 14, .h = 10 },
    { .x = 80, .y = 41, .w = 14, .h = 10 },
    { .x = 66, .y = 41, .w = 14, .h = 10 },
    // Player 2 gate
    { .x = 61, .y = 24, .w = 10, .h = 14 },
    // Player 1 towers
    { .x =  9, .y = 10, .w =  6, .h =  6 },
    { .x =  8, .y = 43, .w =  6, .h =  6 },
    { .x = 21, .y = 18, .w =  6, .h =  6 },
    { .x = 21, .y = 36, .w =  6, .h =  6 },
    // Player 2 towers
    { .x = 73, .y = 10, .w =  6, .h =  6 },
    { .x = 76, .y = 43, .w =  6, .h =  6 },
    { .x = 64, .y = 15, .w =  6, .h =  6 },
    { .x = 65, .y = 36, .w =  6, .h =  6 },
};
const uint8_t obstacle_flags[NUMBER_OF_OBSTACLES] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x82,
    0x04, 0x04, 0x04, 0x04, 0x84,
    0x05, 0x05, 0x05, 0x05, 0x85,
    0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09
};
uint8_t obstacle_stone_quantity[NUMBER_OF_OBSTACLES];
struct robot_model robots[2];

void init_model(void)
{
    memset(&robots, 0, 2 * sizeof(struct robot_model));

    robots[0].control_keys[CONTROL_KEY_UP] = KEY_Z;
    robots[0].control_keys[CONTROL_KEY_DOWN] = KEY_S;
    robots[0].control_keys[CONTROL_KEY_RIGHT] = KEY_D;
    robots[0].control_keys[CONTROL_KEY_LEFT] = KEY_Q;
    robots[0].control_keys[CONTROL_KEY_ACTION] = KEY_A;

    robots[1].control_keys[CONTROL_KEY_UP] = KEY_UP;
    robots[1].control_keys[CONTROL_KEY_DOWN] = KEY_DOWN;
    robots[1].control_keys[CONTROL_KEY_RIGHT] = KEY_RIGHT;
    robots[1].control_keys[CONTROL_KEY_LEFT] = KEY_LEFT;
    robots[1].control_keys[CONTROL_KEY_ACTION] = KEY_COPY;

    robots[0].position.x = 0;
    robots[0].position.y = ROBOT_STARTING_POSITION_V;
    robots[0].orientation = ORIENTATION_EAST;

    robots[1].position.x = TABLE_EDGE_U;
    robots[1].position.y = ROBOT_STARTING_POSITION_V;
    robots[1].orientation = ORIENTATION_WEST;
}
