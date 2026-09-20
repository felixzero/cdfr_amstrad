#include "game_model.h"

#include <string.h>

struct game_model game_model;

void reset_game_model(void)
{
    memset(&game_model, 0, sizeof(game_model));

    // Robot 1 starts in its zone facing the opponent
    game_model.robot_1.u = INITIAL_ROBOT_1_U;
    game_model.robot_1.v = INITIAL_ROBOT_1_V;
    game_model.robot_1.orientation = ROBOT_ORIENTATION_E;

    game_model.robot_2.u = INITIAL_ROBOT_2_U;
    game_model.robot_2.v = INITIAL_ROBOT_2_V;
    game_model.robot_2.orientation = ROBOT_ORIENTATION_W;

    // Quarries start with 3 stones each
    for (uint8_t i = 0; i < NUMBER_OF_QUARRIES; ++i) {
        game_model.quarry_stone_quantity[i] = INITIAL_NUMBER_OF_STONES_PER_QUARRY;
    }
}
