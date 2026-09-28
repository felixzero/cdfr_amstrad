#pragma once

#include <stdint.h>
#include "rect.h"

#define ORIENTATION_EAST                0
#define ORIENTATION_SOUTH               1
#define ORIENTATION_WEST                2
#define ORIENTATION_NORTH               3

#define TABLE_EDGE_U                    90
#define TABLE_EDGE_V                    58

#define ROBOT_STARTING_POSITION_V       30
#define ROBOT_HITBOX                    8
#define ROTATION_TIMER_MASK             0x0F
#define MINING_TIMER_OUT                4
#define ROBOT_MINING_DISTANCE           4

#define NUMBER_OF_QUARRIES              10
#define NUMBER_OF_WALLS                 10
#define NUMBER_OF_TOWERS                8
#define INITIAL_NUMBER_OF_STONES        3

#define COLLISION_OUT_OF_TABLE          -1
#define COLLISION_OTHER_ROBOT           -2
#define COLLISION_NOTHING               0
#define COLLISION_OBSTACLE_START        1

#define NUMBER_OF_OBSTACLES             (NUMBER_OF_QUARRIES + NUMBER_OF_WALLS + NUMBER_OF_TOWERS)

#define OBSTACLE_FLAG_PLAYER_ID         0x03
#define OBSTACLE_PLAYER_ID_0            0x00
#define OBSTACLE_PLAYER_ID_1            0x01
#define OBSTACLE_PLAYER_ID_NONE         0x02

#define OBSTACLE_FLAG_TYPE              0x0C
#define OBSTACLE_TYPE_QUARRY            0x00
#define OBSTACLE_TYPE_WALL              0x04
#define OBSTACLE_TYPE_TOWER             0x08

#define OBSTACLE_FLAG_SOUTH_ORIENTED    (1 << 7)

#define IS_ORIENTED_EAST(x)             !(obstacle_flags[x] & OBSTACLE_FLAG_SOUTH_ORIENTED)

struct robot_model {
    struct point position;
    uint8_t orientation;
    uint8_t carried_stones;
    uint8_t action_timer;
    uint32_t control_keys[5];
};

extern const struct rect obstacles[];
extern const uint8_t obstacle_flags[NUMBER_OF_OBSTACLES];
extern uint8_t obstacle_stone_quantity[NUMBER_OF_OBSTACLES];
extern struct robot_model robots[2];

void init_model(void);
