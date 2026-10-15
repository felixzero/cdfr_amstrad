#include "controller.h"
#include "model.h"
#include "view.h"
#include "inputs.h"
#include "ui.h"
#include "print.h"
#include "score.h"
#include "audio.h"

#include <string.h>

uint8_t game_state = GAME_STATE_INIT;

static void update_controller_init(void);
static void update_controller_wait_ready(void);
static void update_controller_321(void);
static void update_controller_play(void);
static void update_controller_finished(void);
static int8_t check_collisions(uint8_t robot_id);
static void displace_with_orientation(struct point *position, uint8_t orientation, int8_t increment);
static void move_pamis(uint8_t iteration);
static void launch_ball(uint8_t player_id);
static void update_flying_balls(void);

static uint8_t frame_count = 0, player_ready_flags = 0;

void init_controller(void)
{
    // Copy background to video ram
    memcpy(VIDEO_RAM_START, (void*)0x4000, 0x4000);
    swap_buffers();

    init_view();
}


void update_controller(void)
{
    switch (game_state) {
        case GAME_STATE_INIT:
        update_controller_init();
        break;

        case GAME_STATE_WAIT_READY:
        update_controller_wait_ready();
        break;

        case GAME_STATE_321:
        update_controller_321();
        break;

        case GAME_STATE_PLAY:
        update_controller_play();
        break;

        case GAME_STATE_FINISHED:
        update_controller_finished();
        break;
    }
    update_view();
}


static void update_controller_init(void)
{
    if (frame_count >= INIT_FRAME_DELAY) {
        user_messages[0] = PLAYER_MESSAGE_READY;
        user_messages[1] = PLAYER_MESSAGE_READY;
        question = QUESTION_READY;

        game_state = GAME_STATE_WAIT_READY;
    }

    frame_count++;
}


static void update_controller_wait_ready(void)
{
    static uint8_t i;

    if (player_ready_flags == 0x03) {
        clock_digits[0] = 0;
        clock_digits[1] = 3;
        question = QUESTION_WILL_START;
        frame_count = 0;

        game_state = GAME_STATE_321;
    }

    get_keypress();

    for (i = 0; i < 2; ++i) {
        if (is_key_pressed(i, check_key_action)) {
            player_ready_flags |= (1 << i);
            user_messages[i] = PLAYER_MESSAGE_NONE;
        }
    }
}


static void update_controller_321(void)
{
    if (frame_count >= COUNT_DOWN_DELAY) {
        if (decrement_game_clock()) {
            question = QUESTION_NONE;
            clock_digits[0] = 9;
            clock_digits[1] = 9;

            game_state = GAME_STATE_PLAY;
            init_audio_player(MAIN_MUSIC_START);
        }
        frame_count = 0;
    }

    frame_count++;
}

static void update_controller_finished(void)
{
    display_scores();

    while (true) {
        get_keypress();
        if (is_key_pressed(0, check_key_action)) {
            // Restart the game, the violent way
            __asm
            jp _init
            __endasm;
        }
    }
}


static void update_controller_play(void)
{
    static uint8_t increment;
    static uint8_t i;
    static struct point p;
    static uint8_t frame_counter = 0;
    static uint8_t pami_motions = 0;
    static bool change_orientation;
    static int8_t obstacle_id;
    static uint8_t mining_interaction;
    static struct robot_model *robot;
    static bool is_player_firing;

    change_orientation = false;
    get_keypress();

    for (i = 0; i < 2; ++i) {
        robot = &robots[i];
        is_player_firing = (
            ((clock_digits[0] == 0) || ((clock_digits[0] == 1) && (clock_digits[1] <= 5)))
            && (launched_balls[i] < NUMBER_OF_BALLS_PER_PLAYER)
        );

        mining_interaction = MINING_INTERACTION_NONE;

        p.x = robot->position.x + ROBOT_HALF_WIDTH;
        p.y = robot->position.y + ROBOT_HALF_WIDTH;
        displace_with_orientation(&p, robot->orientation, ROBOT_MINING_DISTANCE);

        if (is_player_firing) {
            user_messages[i] = PLAYER_MESSAGE_FIRE;
        } else {
            obstacle_id = check_mining_interaction(&p);
            if (obstacle_id >= 0) {
                mining_interaction = manage_mining_interaction(obstacle_id, i);
                user_messages[i] = mining_interaction;
            } else {
                user_messages[i] = PLAYER_MESSAGE_NONE;
            }
        }

        bool up_pressed = is_key_pressed(i, check_key_up);

        if (is_key_pressed(i, check_key_right)) {
            if (robot->action_timer == 0) {
                robot->orientation++;
                robot->orientation &= 0x03;
                change_orientation = true;
            }
            robot->action_timer++;
            robot->action_timer &= ROTATION_TIMER_MASK;
        } else if (is_key_pressed(i, check_key_left)) {
            if (robot->action_timer == 0) {
                robot->orientation--;
                robot->orientation &= 0x03;
                change_orientation = true;
            }
            robot->action_timer++;
            robot->action_timer &= ROTATION_TIMER_MASK;
        } else if (up_pressed || is_key_pressed(i, check_key_down)) {
            increment = up_pressed ? 2 : -2;
            displace_with_orientation((struct point*)&robot->position, robot->orientation, increment);
            if (check_collisions(i)) {
                displace_with_orientation((struct point*)&robot->position, robot->orientation, -increment);
            }
            robot->action_timer = 0;
        } else if (is_key_pressed(i, check_key_action)) {
            if ((robot->action_timer == MINING_TIMER_OUT)) {
                if (is_player_firing) {
                    launch_ball(i);
                } else {
                    switch (mining_interaction) {
                        case MINING_INTERACTION_MINE:
                        obstacle_stone_quantity[obstacle_id]--;
                        robot->carried_stones++;
                        user_bricks[i] = robot->carried_stones;
                        update_obstacle_sprite(obstacle_id);
                        play_sound_effect(SOUND_EFFECT_MINE);
                        break;

                        case MINING_INTERACTION_BUILD:
                        obstacle_stone_quantity[obstacle_id]++;
                        robot->carried_stones--;
                        user_bricks[i] = robot->carried_stones;
                        update_obstacle_sprite(obstacle_id);
                        play_sound_effect(SOUND_EFFECT_BUILD);
                        break;

                        case MINING_INTERACTION_PICK:
                        grail_locations[i] = -1;
                        robot->carry_grail = true;
                        update_obstacle_sprite(obstacle_id);
                        break;

                        case MINING_INTERACTION_PUT:
                        grail_locations[i] = obstacle_id;
                        robot->carry_grail = false;
                        update_obstacle_sprite(obstacle_id);
                        play_sound_effect(SOUND_EFFECT_GRAIL);
                        break;
                    }
                }

                robot->action_timer = 0;
            } else {
                robot->action_timer++;
            }
        } else {
            robot->action_timer = 0;
        }

        update_robot_sprite(i, change_orientation);
    }

    // Move PAMIs - only in the last 10s
    if (clock_digits[0] == 0) {
        move_pamis(pami_motions);
        update_pami_sprite(PAMI_ID_START);
        update_pami_sprite(PAMI_ID_START + 1);
        pami_motions++;
    }

    if ((clock_digits[0] == 0) || (clock_digits[0] == 1)) {
        update_flying_balls();
    }

    // Update clock display
    frame_counter++;
    if (frame_counter >= FRAME_PER_SECONDS) {
        frame_counter = 0;
        if (decrement_game_clock()) {
            clock_digits[0] = 0;
            clock_digits[1] = 0;
            frame_counter = 0;
            game_state = GAME_STATE_FINISHED;
            stop_audio_player();
        }
    }
}


int8_t check_collisions(uint8_t robot_id)
{
    static uint8_t i;
    static struct robot_model *other_robot, *robot;
    static const struct rect table_edge_collision_rect = {
        .x = 0,
        .y = 0,
        .w = TABLE_EDGE_U - 2 * ROBOT_HALF_WIDTH,
        .h = TABLE_EDGE_V - 2 * ROBOT_HALF_WIDTH
    };

    robot = &robots[robot_id];
    other_robot = &robots[1 - robot_id];

    if (!rect_contains(&table_edge_collision_rect, (struct point*)&robot->position)) {
        return COLLISION_OUT_OF_TABLE;
    }

    if (rect_intersects(&robot->position, &other_robot->position)) {
        return COLLISION_OTHER_ROBOT;
    }

    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        if (
            rect_intersects(&obstacles[i], &robot->position)
            && ((obstacle_stone_quantity[i] != 0)
            || ((uint8_t)grail_locations[0] == i)
            || ((uint8_t)grail_locations[1] == i))
        ) {
            return COLLISION_OBSTACLE_START + i;
        }
    }

    return COLLISION_NOTHING;
}


int8_t check_mining_interaction(struct point *uv)
{
    static uint8_t i;

    // Reverse order favors towers over walls
    // Otherwise towers are nearly impossible to build
    for (i = PAMI_ID_START - 1; i != 0xFF; --i) {
        if (rect_contains(&obstacles[i], uv)) {
            return i;
        }
    }

    return -1;
}


static void displace_with_orientation(struct point *position, uint8_t orientation, int8_t increment)
{
    switch (orientation) {
        case ORIENTATION_EAST:
        position->x += increment;
        return;

        case ORIENTATION_WEST:
        position->x -= increment;
        return;

        case ORIENTATION_SOUTH:
        position->y += increment;
        return;

        case ORIENTATION_NORTH:
        position->y -= increment;
        return;
    }
}



uint8_t manage_mining_interaction(uint8_t obstacle_id, uint8_t robot_id)
{
    static uint8_t obstacle_type, player_id;
    static struct robot_model *robot;

    robot = &robots[robot_id];

    obstacle_type = obstacle_flags[obstacle_id] & OBSTACLE_FLAG_TYPE;
    player_id = obstacle_flags[obstacle_id] & OBSTACLE_FLAG_PLAYER_ID;

    // Pick the grail from its holder
    if ((obstacle_type == OBSTACLE_TYPE_GRAIL_HOLDER) && (player_id == robot_id)) {
        if ((uint8_t)grail_locations[player_id] == obstacle_id) {
            return MINING_INTERACTION_PICK;
        }
    }

    // Mining situation
    if ((obstacle_type == OBSTACLE_TYPE_QUARRY) || (player_id != robot_id)) {
        if (obstacle_stone_quantity[obstacle_id] > 0 && robot->carried_stones < 3) {
            return MINING_INTERACTION_MINE;
        }
    }

    if (player_id == robot_id) {
        // Deposition situation
        if ((robot->carry_grail) && (obstacle_type == OBSTACLE_TYPE_WALL) && (obstacle_stone_quantity[obstacle_id] > 0)) {
            return MINING_INTERACTION_PUT;
        } else if ( // Construction situation
            (
                (obstacle_type == OBSTACLE_TYPE_WALL)
                && (robot->carried_stones > 0)
                && (obstacle_stone_quantity[obstacle_id] < 3)
            )
            || (
                (obstacle_type == OBSTACLE_TYPE_TOWER)
                && (robot->carried_stones > 0 && obstacle_stone_quantity[obstacle_id] < 1)
            )
        ) {
            if (
                (!rect_intersects(&obstacles[obstacle_id], &robots[0].position))
                && (!rect_intersects(&obstacles[obstacle_id], &robots[1].position))
            ) {
                return MINING_INTERACTION_BUILD;
            }
        }
    }

    return MINING_INTERACTION_NONE;
}


static void move_pamis(uint8_t iteration)
{
    static struct rect *obstacle;
    static uint8_t i;

    for (i = 0; i < NUMBER_OF_PAMIS; ++i) {
        if (!pami_stuck[i]) {
            obstacle = &obstacles[PAMI_ID_START + i];

            // PAMIs perform a Z shape motion
            if (iteration < PAMI_MOTION_KNEE_1) {
                obstacle->x += i ? -1 : 1;
            } else if (iteration < PAMI_MOTION_KNEE_2) {
                obstacle->y += i ? -1 : 1;
            } else if (iteration < PAMI_MOTION_KNEE_3) {
                obstacle->x += i ? -1 : 1;
            }

            // PAMI stuck by robot
            if (
                rect_intersects(obstacle, &robots[0].position)
                || rect_intersects(obstacle, &robots[1].position)
            ) {
                pami_stuck[i] = true;
            }

            // PAMI stuck by unmined quarry
            if (
                (obstacle_stone_quantity[PAMI_0_POTENTIAL_QUARRY] > 0 && rect_intersects(obstacle, &obstacles[PAMI_0_POTENTIAL_QUARRY]))
                || (obstacle_stone_quantity[PAMI_1_POTENTIAL_QUARRY] > 0 && rect_intersects(obstacle, &obstacles[PAMI_1_POTENTIAL_QUARRY]))
            ) {
                pami_stuck[i] = true;
            }
        }
    }
}


static void launch_ball(uint8_t player_id)
{
    static uint8_t i;

    i = launched_balls[player_id];
    if (i == NUMBER_OF_BALLS_PER_PLAYER) {
        return;
    }
    if (player_id) {
        i += NUMBER_OF_BALLS_PER_PLAYER;
    }

    balls[i].position.x = robots[player_id].position.x;
    balls[i].position.y = robots[player_id].position.y;
    balls[i].orientation = robots[player_id].orientation;
    displace_with_orientation(&balls[i].position, balls[i].orientation, ROBOT_MINING_DISTANCE);
    balls[i].flags |= BALL_FLAGS_LAUNCHED_MASK;

    launched_balls[player_id]++;
}


static void update_flying_balls(void)
{
    static uint8_t i, j;

    for (i = 0; i < NUMBER_OF_BALLS; ++i) {
        if (!(balls[i].flags & BALL_FLAGS_LAUNCHED_MASK)) {
            continue;
        }

        if (balls[i].damping_factor < MAX_BALL_DAMPING_FACTOR) {
            displace_with_orientation(
                &balls[i].position,
                balls[i].orientation,
                (1 << (MAX_BALL_DAMPING_FACTOR - 1)) >> balls[i].damping_factor
            );

            balls[i].parabolic_index++;
            if (balls[i].parabolic_index == BALL_PARABOLA_LENGTH) {
                balls[i].parabolic_index = 0;
                balls[i].damping_factor++;
                play_sound_effect(SOUND_EFFECT_BOINK);
            }

            // Out of table balls are lost
            if (
                (balls[i].position.x & 0x80)
                || (balls[i].position.y & 0x80)
                || (balls[i].position.x > TABLE_EDGE_U)
                || (balls[i].position.y > TABLE_EDGE_V)
            ) {
                balls[i].flags &= ~BALL_FLAGS_LAUNCHED_MASK;
            }
        }
        
        update_ball_sprite(i);
    }
}
