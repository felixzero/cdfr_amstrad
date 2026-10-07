#include "model.h"
#include "controller.h"
#include "inputs.h"

#include <string.h>

uint8_t obstacle_stone_quantity[NUMBER_OF_OBSTACLES];
struct robot_model robots[2] = {
    { .position = { .x = 0, .y = ROBOT_STARTING_POSITION_V }, .orientation = ORIENTATION_EAST },
    { .position = { .x = TABLE_EDGE_U, .y = ROBOT_STARTING_POSITION_V }, .orientation = ORIENTATION_WEST },
};

int8_t grail_locations[2] = { GRAIL_HOLDERS_ID_START, GRAIL_HOLDERS_ID_START + 1 };
