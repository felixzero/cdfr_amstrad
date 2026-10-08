#pragma once

#define POSITION_X_ORIGIN               58
#define POSITION_Y_ORIGIN               8

#define QUARRY_DISPLAY_OFFSET_U         8
#define QUARRY_DISPLAY_OFFSET_V         7
#define QUARRY_Z_INDEX_OFFSET           3

#define WALL_DISPLAY_OFFSET_U           8
#define WALL_DISPLAY_OFFSET_V           7
#define WALL_Z_INDEX_OFFSET             8

#define TOWER_DISPLAY_OFFSET_U          6
#define TOWER_DISPLAY_OFFSET_V          3

#define GRAIL_DISPLAY_OFFSET_U          8
#define GRAIL_DISPLAY_OFFSET_V          5
#define GRAIL_Z_INDEX_OFFSET            8
#define GRAIL_DISPLAY_OFFSET_Y_STONE    1

#define PAMI_DISPLAY_OFFSET_U           8
#define PAMI_DISPLAY_OFFSET_V           5
#define PAMI_Z_INDEX_OFFSET             8

#include "sprites.h"
#include "model.h"

void init_view(void);

void update_view(void);

void update_obstacle_sprite(uint8_t obstacle_id);

void update_robot_sprite(uint8_t robot_id, bool change_orientation);

void update_pami_sprite(uint8_t obstacle_id);

// Remove all sprites and UI elements, and return to single buffer operation
void clear_view(void);
