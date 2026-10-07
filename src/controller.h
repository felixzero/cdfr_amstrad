#pragma once

#include "rect.h"
#include <stdint.h>

#define CONTROL_KEY_UP                  0
#define CONTROL_KEY_DOWN                1
#define CONTROL_KEY_RIGHT               2
#define CONTROL_KEY_LEFT                3
#define CONTROL_KEY_ACTION              4

#define FRAME_PER_SECONDS               12

#define INIT_FRAME_DELAY                (1 * FRAME_PER_SECONDS)
#define COUNT_DOWN_DELAY                (1 * FRAME_PER_SECONDS)

enum {
    GAME_STATE_INIT,
    GAME_STATE_WAIT_READY,
    GAME_STATE_321,
    GAME_STATE_PLAY,
    GAME_STATE_FINISHED
};

enum {
    MINING_INTERACTION_NONE,
    MINING_INTERACTION_MINE,
    MINING_INTERACTION_BUILD,
    MINING_INTERACTION_PICK,
    MINING_INTERACTION_PUT,
};

void init_controller(void);
void update_controller(void);
int8_t check_collisions(struct point *uv, uint8_t robot_id);
int8_t check_mining_interaction(struct point *uv);
void displace_robot(struct point *destination, uint8_t robot_id, int8_t increment);
uint8_t manage_mining_interaction(uint8_t obstacle_id, uint8_t robot_id);
