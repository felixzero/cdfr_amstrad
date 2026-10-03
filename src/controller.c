#include "controller.h"
#include "model.h"
#include "view.h"
#include "inputs.h"
#include "ui.h"
#include "print.h"
#include "score.h"

#include <string.h>

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
    MINING_INTERACTION_BUILD
};

static uint8_t game_state;

static void update_controller_init(void);
static void update_controller_wait_ready(void);
static void update_controller_321(void);
static void update_controller_play(void);
static void update_controller_finished(void);

static uint8_t frame_count, player_ready_flags;

void init_controller(void)
{
    // Copy background to video ram
    memcpy(VIDEO_RAM_START, (void*)0x4000, 0x4000);
    swap_buffers();

    game_state = GAME_STATE_INIT;
    frame_count = 0;
    player_ready_flags = 0;

    init_model();
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
    static bool change_orientation;
    static int8_t obstacle_id;
    static uint8_t mining_interaction;
    static struct robot_model *robot;

    change_orientation = false;
    get_keypress();

    for (i = 0; i < 2; ++i) {
        robot = &robots[i];

        mining_interaction = MINING_INTERACTION_NONE;
        displace_robot(&p, i, ROBOT_MINING_DISTANCE);
        obstacle_id = check_mining_interaction(&p);
        if (obstacle_id >= 0) {
            mining_interaction = manage_mining_interaction(obstacle_id, i);

            switch (mining_interaction) {
                case MINING_INTERACTION_MINE:
                user_messages[i] = PLAYER_MESSAGE_MINE;
                break;

                case MINING_INTERACTION_BUILD:
                user_messages[i] = PLAYER_MESSAGE_BUILD;
                break;

                default:
                user_messages[i] = PLAYER_MESSAGE_NONE;
                break;
            }
        }

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
        } else if (is_key_pressed(i, check_key_up) || is_key_pressed(i, check_key_down)) {
            increment = is_key_pressed(i, check_key_up) ? 2 : -2;
            displace_robot(&p, i, increment);

            if (!check_collisions(&p, i)) {
                robot->position.x = p.x;
                robot->position.y = p.y;
            }
        } else if (is_key_pressed(i, check_key_action)) {
            if ((robot->action_timer == MINING_TIMER_OUT)) {
                switch (mining_interaction) {
                    case MINING_INTERACTION_MINE:
                    obstacle_stone_quantity[obstacle_id]--;
                    robot->carried_stones++;
                    update_obstacle_sprite(obstacle_id);
                    break;

                    case MINING_INTERACTION_BUILD:
                    obstacle_stone_quantity[obstacle_id]++;
                    robot->carried_stones--;
                    update_obstacle_sprite(obstacle_id);
                    break;
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

    // Update clock display
    frame_counter++;
    if (frame_counter >= FRAME_PER_SECONDS) {
        frame_counter = 0;
        if (decrement_game_clock()) {
            //question = QUESTION_FINISHED;
            clock_digits[0] = 0;
            clock_digits[1] = 0;
            frame_counter = 0;
            game_state = GAME_STATE_FINISHED;
        }
    }
}


int8_t check_collisions(struct point *uv, uint8_t robot_id)
{
    static struct rect r;
    static uint8_t i;
    static struct robot_model *other_robot;

    other_robot = &robots[1 - robot_id];

    r.w = ROBOT_HITBOX * 2;
    r.h = ROBOT_HITBOX * 2;
    r.x = other_robot->position.x - ROBOT_HITBOX;
    r.y = other_robot->position.y - ROBOT_HITBOX;

    // Out of table
    if ((uv->x & 0x80) || (uv->y & 0x80) || (uv->x > TABLE_EDGE_U) || (uv->y > TABLE_EDGE_V)) {
        return COLLISION_OUT_OF_TABLE;
    }

    // Robot collision
    if (rect_contains(&r, uv)) {
        return COLLISION_OTHER_ROBOT;
    }

    // Obstacle collision
    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        if (rect_contains(&obstacles[i], uv) && (obstacle_stone_quantity[i] != 0)) {
            return COLLISION_OBSTACLE_START + i;
        }
    }

    return COLLISION_NOTHING;
}


int8_t check_mining_interaction(struct point *uv)
{
    static uint8_t i;

    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        if (rect_contains(&obstacles[i], uv)) {
            return i;
        }
    }

    return -1;
}


void displace_robot(struct point *destination, uint8_t robot_id, int8_t increment)
{
    static struct robot_model *robot;

    robot = &robots[robot_id];

    destination->x = robot->position.x;
    destination->y = robot->position.y;

    switch (robot->orientation) {
        case ORIENTATION_EAST:
        destination->x += increment;
        return;

        case ORIENTATION_WEST:
        destination->x -= increment;
        return;

        case ORIENTATION_SOUTH:
        destination->y += increment;
        return;

        case ORIENTATION_NORTH:
        destination->y -= increment;
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

    // Mining situation
    if ((obstacle_type == OBSTACLE_TYPE_QUARRY) || (player_id != robot_id)) {
        if (obstacle_stone_quantity[obstacle_id] > 0 && robot->carried_stones < 3) {
            return MINING_INTERACTION_MINE;
        }
    }

    // Construction situation
    if (player_id == robot_id) {
        if (
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
            return MINING_INTERACTION_BUILD;
        }
    }

    return MINING_INTERACTION_NONE;
}

