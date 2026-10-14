#pragma once

#define POSITION_X_ORIGIN              64
#define POSITION_Y_ORIGIN              22

// Position of the north-west corner on the bottom horizontal plane of the object
// As displayed in (X, Y) coordinate in the sprite
#define EAST_BLOCK_OFFSET_X             6
#define EAST_BLOCK_OFFSET_Y             12
#define SOUTH_BLOCK_OFFSET_X            6
#define SOUTH_BLOCK_OFFSET_Y            11
#define BALL_OFFSET_X                   2
#define BALL_OFFSET_Y                   2
#define GRAIL_OFFSET_X                  2
#define GRAIL_OFFSET_Y                  10
#define PAMI_OFFSET_X                   2
#define PAMI_OFFSET_Y                   6
#define ROBOT_OFFSET_X                  6
#define ROBOT_OFFSET_Y                  15
#define TOWER_OFFSET_X                  2
#define TOWER_OFFSET_Y                  12

#define GRAIL_DISPLAY_OFFSET_X          0
#define GRAIL_DISPLAY_OFFSET_Y          4
#define GRAIL_DISPLAY_OFFSET_Y_STONE    2


#include "sprites.h"
#include "model.h"

void init_view(void);

void update_view(void);

void update_obstacle_sprite(uint8_t obstacle_id);

void update_robot_sprite(uint8_t robot_id, bool change_orientation);

void update_pami_sprite(uint8_t obstacle_id);

void update_ball_sprite(uint8_t ball_id);

// Remove all sprites and UI elements, and return to single buffer operation
void clear_view(void);
