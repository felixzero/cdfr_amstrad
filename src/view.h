#pragma once

#define POSITION_X_ORIGIN               58
#define POSITION_Y_ORIGIN               8

#define QUARRY_DISPLAY_OFFSET_U         8
#define QUARRY_DISPLAY_OFFSET_V         7
#define QUARRY_Z_INDEX_OFFSET           8

#define WALL_DISPLAY_OFFSET_U           8
#define WALL_DISPLAY_OFFSET_V           7
#define WALL_Z_INDEX_OFFSET             8

#define TOWER_DISPLAY_OFFSET_U          6
#define TOWER_DISPLAY_OFFSET_V          3

#include "sprites.h"
#include "model.h"

void init_view(void);

void update_view(void);

void update_obstacle_sprite(uint8_t obstacle_id);

void update_robot_sprite(uint8_t robot_id, bool change_orientation);
