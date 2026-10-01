#include "model.h"
#include "controller.h"
#include "inputs.h"

#include <string.h>

uint8_t obstacle_stone_quantity[NUMBER_OF_OBSTACLES];
struct robot_model robots[2];

void init_model(void)
{
    memset(&robots, 0, 2 * sizeof(struct robot_model));

    robots[0].position.x = 0;
    robots[0].position.y = ROBOT_STARTING_POSITION_V;
    robots[0].orientation = ORIENTATION_EAST;

    robots[1].position.x = TABLE_EDGE_U;
    robots[1].position.y = ROBOT_STARTING_POSITION_V;
    robots[1].orientation = ORIENTATION_WEST;
}
