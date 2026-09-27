#include "model_view.h"

#include "sprites.h"
#include "inputs.h"
#include "sprite_assets.h"
#include "game_clock.h"

#include <string.h>

static const struct rect obstacles[] = {
    // W-E oriented quarries
    { .x =  4, .y = -5, .w = 14, .h = 10 },
    { .x = 26, .y = -5, .w = 14, .h = 10 },
    { .x =  4, .y = 53, .w = 14, .h = 10 },
    { .x = 26, .y = 53, .w = 14, .h = 10 },
    { .x = 74, .y = -5, .w = 14, .h = 10 },
    { .x = 52, .y = -5, .w = 14, .h = 10 },
    { .x = 74, .y = 53, .w = 14, .h = 10 },
    { .x = 52, .y = 53, .w = 14, .h = 10 },
    // N-S oriented quarries
    { .x = 38, .y = 11, .w = 10, .h = 14 },
    { .x = 39, .y = 38, .w = 10, .h = 14 },
    // W-E player 1 walls
    { .x = -3, .y =  8, .w = 14, .h = 10 },
    { .x = 13, .y =  8, .w = 14, .h = 10 },
    { .x = -4, .y = 41, .w = 14, .h = 10 },
    { .x = 12, .y = 41, .w = 14, .h = 10 },
    // Player 1 gate
    { .x = 16, .y = 25, .w = 10, .h = 14 },
    // W-E player 2 walls
    { .x = 80, .y =  7, .w = 14, .h = 10 },
    { .x = 65, .y =  8, .w = 14, .h = 10 },
    { .x = 80, .y = 41, .w = 14, .h = 10 },
    { .x = 66, .y = 41, .w = 14, .h = 10 },
    // Player 2 gate
    { .x = 61, .y = 24, .w = 10, .h = 14 },
    // Player 1 towers
    { .x =  9, .y = 10, .w =  6, .h =  6 },
    { .x =  8, .y = 43, .w =  6, .h =  6 },
    { .x = 21, .y = 18, .w =  6, .h =  6 },
    { .x = 21, .y = 36, .w =  6, .h =  6 },
    // Player 2 towers
    { .x = 73, .y = 10, .w =  6, .h =  6 },
    { .x = 76, .y = 43, .w =  6, .h =  6 },
    { .x = 64, .y = 15, .w =  6, .h =  6 },
    { .x = 65, .y = 36, .w =  6, .h =  6 },
};
static const uint8_t obstacle_flags[NUMBER_OF_OBSTACLES] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x82,
    0x04, 0x04, 0x04, 0x04, 0x84,
    0x05, 0x05, 0x05, 0x05, 0x85,
    0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09
};

static sprite_handle_t obstacle_sprites[NUMBER_OF_OBSTACLES];
static uint8_t obstacle_stone_quantity[NUMBER_OF_OBSTACLES];

struct robot_model {
    struct point position;
    uint8_t orientation;
    uint8_t carried_stones;
    uint8_t action_timer;

    sprite_handle_t sprite;
    uint8_t *sprite_assets[4];
    uint32_t control_keys[5];
};

static struct robot_model robots[2];

static void create_quarry_sprite(uint8_t obstacle_id);
static void create_wall_sprite(uint8_t obstacle_id);
static void create_tower_sprite(uint8_t obstacle_id);
static inline void game_uv_to_screen_xy(struct point *xy, const struct point *uv);
static int8_t check_collisions(struct point *uv, uint8_t robot_id);
static int8_t check_mining_interaction(struct point *uv);
static void displace_robot(struct point *destination, uint8_t robot_id, int8_t increment);
static void manage_mining_interaction(uint8_t obstacle_id, uint8_t robot_id);
static void update_obstacle_sprite(uint8_t obstacle_id);

void init_model_view(void)
{
    uint8_t i;
    struct point p;
    struct rect r;

    memset(&robots, 0, 2 * sizeof(struct robot_model));

    robots[0].sprite_assets[ORIENTATION_EAST] = asset_robot_1e;
    robots[0].sprite_assets[ORIENTATION_SOUTH] = asset_robot_1s;
    robots[0].sprite_assets[ORIENTATION_WEST] = asset_robot_1w;
    robots[0].sprite_assets[ORIENTATION_NORTH] = asset_robot_1n;
    robots[0].control_keys[CONTROL_KEY_UP] = KEY_Z;
    robots[0].control_keys[CONTROL_KEY_DOWN] = KEY_S;
    robots[0].control_keys[CONTROL_KEY_RIGHT] = KEY_D;
    robots[0].control_keys[CONTROL_KEY_LEFT] = KEY_Q;
    robots[0].control_keys[CONTROL_KEY_ACTION] = KEY_A;

    robots[1].sprite_assets[ORIENTATION_EAST] = asset_robot_2e;
    robots[1].sprite_assets[ORIENTATION_SOUTH] = asset_robot_2s;
    robots[1].sprite_assets[ORIENTATION_WEST] = asset_robot_2w;
    robots[1].sprite_assets[ORIENTATION_NORTH] = asset_robot_2n;
    robots[1].control_keys[CONTROL_KEY_UP] = KEY_UP;
    robots[1].control_keys[CONTROL_KEY_DOWN] = KEY_DOWN;
    robots[1].control_keys[CONTROL_KEY_RIGHT] = KEY_RIGHT;
    robots[1].control_keys[CONTROL_KEY_LEFT] = KEY_LEFT;
    robots[1].control_keys[CONTROL_KEY_ACTION] = KEY_COPY;

    robots[0].position.x = 0;
    robots[0].position.y = ROBOT_STARTING_POSITION_V;
    robots[0].orientation = ORIENTATION_EAST;

    robots[1].position.x = TABLE_EDGE_U;
    robots[1].position.y = ROBOT_STARTING_POSITION_V;
    robots[1].orientation = ORIENTATION_WEST;

    // Create robot sprites
    r.w = ASSET_ROBOT_1E_WIDTH;
    r.h = ASSET_ROBOT_1E_HEIGHT;
    game_uv_to_screen_xy(&p, &robots[0].position);
    r.x = p.x;
    r.y = p.y;
    robots[0].sprite = create_sprite(asset_robot_1e, &r);
    set_sprite_z_index(robots[0].sprite, 0);
    game_uv_to_screen_xy(&p, &robots[1].position);
    r.x = p.x;
    r.y = p.y;
    robots[1].sprite = create_sprite(asset_robot_2w, &r);
    set_sprite_z_index(robots[1].sprite, 0);

    // Create obstacle sprites
    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        switch (obstacle_flags[i] & OBSTACLE_FLAG_TYPE) {
            case OBSTACLE_TYPE_QUARRY:
            create_quarry_sprite(i);
            break;

            case OBSTACLE_TYPE_WALL:
            create_wall_sprite(i);
            break;

            case OBSTACLE_TYPE_TOWER:
            create_tower_sprite(i);
            break;
        }
    }

    // Init clock
    init_game_clock();
}


static void create_quarry_sprite(uint8_t obstacle_id)
{
    struct rect r;
    struct point p;

    r.w = ASSET_BLOCK_3E_WIDTH;
    r.h = ASSET_BLOCK_3E_HEIGHT;
    p.x = obstacles[obstacle_id].x + QUARRY_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + QUARRY_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    if (IS_ORIENTED_EAST(obstacle_id)) {
        obstacle_sprites[obstacle_id] = create_sprite(asset_block_3e, &r);
    } else {
        obstacle_sprites[obstacle_id] = create_sprite(asset_block_3s, &r);
    }
    set_sprite_z_index(obstacle_sprites[obstacle_id], r.y / 2 - QUARRY_Z_INDEX_OFFSET / 2);
    trigger_sprite_redraw(obstacle_sprites[obstacle_id]);

    obstacle_stone_quantity[obstacle_id] = INITIAL_NUMBER_OF_STONES;
}


static void create_wall_sprite(uint8_t obstacle_id)
{
    struct rect r;
    struct point p;

    r.w = ASSET_BLOCK_3E_WIDTH;
    r.h = ASSET_BLOCK_3E_HEIGHT;
    p.x = obstacles[obstacle_id].x + WALL_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + WALL_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    if (IS_ORIENTED_EAST(obstacle_id)) {
        obstacle_sprites[obstacle_id] = create_sprite(asset_block_1e, &r);
    } else {
        obstacle_sprites[obstacle_id] = create_sprite(asset_block_1s, &r);
    }
    set_sprite_z_index(obstacle_sprites[obstacle_id], r.y / 2 - WALL_Z_INDEX_OFFSET / 2);
    set_sprite_visibility(obstacle_sprites[obstacle_id], false);
    trigger_sprite_redraw(obstacle_sprites[obstacle_id]);

    obstacle_stone_quantity[obstacle_id] = 0;
}


static void create_tower_sprite(uint8_t obstacle_id)
{
    struct rect r;
    struct point p;

    r.w = ASSET_TOWER_WIDTH;
    r.h = ASSET_TOWER_HEIGHT;
    p.x = obstacles[obstacle_id].x + TOWER_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + TOWER_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    obstacle_sprites[obstacle_id] = create_sprite(asset_tower, &r);
    set_sprite_z_index(obstacle_sprites[obstacle_id], r.y / 2 - WALL_Z_INDEX_OFFSET / 2);
    set_sprite_visibility(obstacle_sprites[obstacle_id], false);
    trigger_sprite_redraw(obstacle_sprites[obstacle_id]);

    obstacle_stone_quantity[obstacle_id] = 0;
}


void update_model_view(void)
{
    static uint32_t keys;
    static uint8_t increment;
    static uint8_t i;
    static struct point p;
    static uint8_t frame_counter = 0;

    keys = get_keypress();

    for (i = 0; i < 2; ++i) {
        if (!(keys & robots[i].control_keys[CONTROL_KEY_RIGHT])) {
            if (robots[i].action_timer == 0) {
                robots[i].orientation++;
                robots[i].orientation &= 0x03;
                change_sprite_asset(robots[i].sprite, robots[i].sprite_assets[robots[i].orientation]);
            }
            robots[i].action_timer++;
            robots[i].action_timer &= ROTATION_TIMER_MASK;
        } else if (!(keys & robots[i].control_keys[CONTROL_KEY_LEFT])) {
            if (robots[i].action_timer == 0) {
                robots[i].orientation--;
                robots[i].orientation &= 0x03;
                change_sprite_asset(robots[i].sprite, robots[i].sprite_assets[robots[i].orientation]);
            }
            robots[i].action_timer++;
            robots[i].action_timer &= ROTATION_TIMER_MASK;
        } else if (!(keys & robots[i].control_keys[CONTROL_KEY_UP]) || !(keys & robots[i].control_keys[CONTROL_KEY_DOWN])) {
            increment = !(keys & robots[i].control_keys[CONTROL_KEY_UP]) ? 2 : -2;
            displace_robot(&p, i, increment);

            if (!check_collisions(&p, i)) {
                robots[i].position.x = p.x;
                robots[i].position.y = p.y;
            }
        } else if (!(keys & robots[i].control_keys[CONTROL_KEY_ACTION])) {
            displace_robot(&p, i, ROBOT_MINING_DISTANCE);
            int8_t obstacle_id = check_mining_interaction(&p);
            if (obstacle_id >= 0) {
                if ((robots[i].action_timer == MINING_TIMER_OUT)) {
                    manage_mining_interaction(obstacle_id, i);
                    robots[i].action_timer = 0;
                } else {
                    robots[i].action_timer++;
                }
            }
        } else {
            robots[i].action_timer = 0;
        }

        game_uv_to_screen_xy(&p, &robots[i].position);
        move_sprite(robots[i].sprite, &p);
        set_sprite_z_index(robots[i].sprite, p.y / 2);
        trigger_sprite_redraw(robots[i].sprite); // 6.5 ms

    }

    draw_sprites();

    // Update clock display
    frame_counter++;
    if (frame_counter >= FRAME_PER_SECONDS) {
        frame_counter = 0;
        decrement_game_clock();
    }
    update_clock_display();

    wait_for_vsync();
    swap_buffers();
}


static inline void game_uv_to_screen_xy(struct point *xy, const struct point *uv)
{
    xy->x = POSITION_X_ORIGIN + uv->x - uv->y;
    xy->y = POSITION_Y_ORIGIN + uv->x + uv->y;
}


static int8_t check_collisions(struct point *uv, uint8_t robot_id)
{
    static struct rect r;
    static uint8_t i;

    r.w = ROBOT_HITBOX * 2;
    r.h = ROBOT_HITBOX * 2;
    r.x = robots[1 - robot_id].position.x - ROBOT_HITBOX;
    r.y = robots[1 - robot_id].position.y - ROBOT_HITBOX;

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

static int8_t check_mining_interaction(struct point *uv)
{
    static uint8_t i;

    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        if (rect_contains(&obstacles[i], uv)) {
            return i;
        }
    }

    return -1;
}


static void displace_robot(struct point *destination, uint8_t robot_id, int8_t increment)
{
    destination->x = robots[robot_id].position.x;
    destination->y = robots[robot_id].position.y;

    switch (robots[robot_id].orientation) {
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


static void manage_mining_interaction(uint8_t obstacle_id, uint8_t robot_id)
{
    static uint8_t obstacle_type, player_id;

    obstacle_type = obstacle_flags[obstacle_id] & OBSTACLE_FLAG_TYPE;
    player_id = obstacle_flags[obstacle_id] & OBSTACLE_FLAG_PLAYER_ID;

    // Mining situation
    if ((obstacle_type == OBSTACLE_TYPE_QUARRY) || (player_id != robot_id)) {
        if (obstacle_stone_quantity[obstacle_id] > 0 && robots[robot_id].carried_stones < 3) {
            obstacle_stone_quantity[obstacle_id]--;
            robots[robot_id].carried_stones++;

            update_obstacle_sprite(obstacle_id);
        }
    }

    // Construction situation
    if (player_id == robot_id) {
        if (
            (
                (obstacle_type == OBSTACLE_TYPE_WALL)
                && (robots[robot_id].carried_stones > 0)
                && (obstacle_stone_quantity[obstacle_id] < 3)
            )
            || (
                (obstacle_type == OBSTACLE_TYPE_TOWER)
                && (robots[robot_id].carried_stones > 0 && obstacle_stone_quantity[obstacle_id] < 1)
            )
        ) {
            obstacle_stone_quantity[obstacle_id]++;
            robots[robot_id].carried_stones--;

            update_obstacle_sprite(obstacle_id);
        }
    }
}


static void update_obstacle_sprite(uint8_t obstacle_id)
{
    static bool is_oriented_east;
    static uint8_t stone_quantity;
    static uint8_t obstacle_type;

    is_oriented_east = IS_ORIENTED_EAST(obstacle_id);
    stone_quantity = obstacle_stone_quantity[obstacle_id];
    obstacle_type = obstacle_flags[obstacle_id] & OBSTACLE_FLAG_TYPE;

    if ((obstacle_type == OBSTACLE_TYPE_QUARRY) || (obstacle_type == OBSTACLE_TYPE_WALL)) {
        switch (obstacle_stone_quantity[obstacle_id]) {
            case 0:
            set_sprite_visibility(obstacle_sprites[obstacle_id], false);
            break;

            case 1:
            change_sprite_asset(obstacle_sprites[obstacle_id], is_oriented_east ? asset_block_1e : asset_block_1s);
            set_sprite_visibility(obstacle_sprites[obstacle_id], true);
            break;

            case 2:
            change_sprite_asset(obstacle_sprites[obstacle_id], is_oriented_east ? asset_block_2e : asset_block_2s);
            set_sprite_visibility(obstacle_sprites[obstacle_id], true);
            break;

            case 3:
            if (obstacle_type == OBSTACLE_TYPE_QUARRY) {
                change_sprite_asset(obstacle_sprites[obstacle_id], is_oriented_east ? asset_block_3e : asset_block_3s);
            } else {
                change_sprite_asset(obstacle_sprites[obstacle_id], is_oriented_east ? asset_block_3e_built : asset_block_3s_built);
            }
            set_sprite_visibility(obstacle_sprites[obstacle_id], true);
            break;
        }
    } else if (obstacle_type == OBSTACLE_TYPE_TOWER) {
        if (obstacle_stone_quantity[obstacle_id] == 0) {
            set_sprite_visibility(obstacle_sprites[obstacle_id], false);
        } else {
            set_sprite_visibility(obstacle_sprites[obstacle_id], true);
        }
    }

    trigger_sprite_redraw(obstacle_sprites[obstacle_id]);
}
