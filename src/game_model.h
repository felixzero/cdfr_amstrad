#pragma once

#include <stdint.h>

#define ROBOT_ORIENTATION_N 0
#define ROBOT_ORIENTATION_E 1
#define ROBOT_ORIENTATION_S 2
#define ROBOT_ORIENTATION_W 3

#define NUMBER_OF_QUARRIES  10
#define NUMBER_OF_WALLS     4
#define NUMBER_OF_TOWERS    4

#define INITIAL_NUMBER_OF_STONES_PER_QUARRY 3
#define INITIAL_ROBOT_1_U 10
#define INITIAL_ROBOT_1_V 100
#define INITIAL_ROBOT_2_U 150
#define INITIAL_ROBOT_2_V 100

/*
Map of quarries:
+-----------------------------+
  [q0] [q1]         [q5] [q4]
              [q8]
              [q9]
  [q2] [q3]         [q7] [q6]
+-----------------------------+

Fort layout:
+--------------------
[w2] [t0] [w1] [t1]
               [w0]
[w4] [t3] [w3] [t2]
+--------------------
*/

struct robot_model {
    uint8_t u;
    uint8_t v;
    uint8_t orientation;
    uint8_t carried_stones;
};

struct fort_model {
    uint8_t wall_stones[NUMBER_OF_WALLS];
    uint8_t built_towers[NUMBER_OF_TOWERS];
};

struct game_model {
    struct robot_model robot_1;
    struct robot_model robot_2;
    struct fort_model fort_1;
    struct fort_model fort_2;
    uint8_t quarry_stone_quantity[NUMBER_OF_QUARRIES];
};

extern struct game_model game_model;

void reset_game_model(void);
