#include "view.h"

#include "sprites.h"
#include "inputs.h"
#include "sprite_assets.h"
#include "ui.h"
#include "controller.h"
#include "model.h"

#include <string.h>

struct robot_view {
    sprite_handle_t sprite;
    uint8_t *sprite_assets[4];
};

struct robot_view robot_views[NUMBER_OF_PLAYERS] = {
    { .sprite = 0, .sprite_assets = { asset_robot_1e, asset_robot_1s, asset_robot_1w, asset_robot_1n }},
    { .sprite = 0, .sprite_assets = { asset_robot_2e, asset_robot_2s, asset_robot_2w, asset_robot_2n }},
};

struct obstacle_3d_model {
    uint8_t asset_width;
    uint8_t asset_height;
    uint8_t z_index_off;
    uint8_t offset_x;
    uint8_t offset_y;
    bool default_visibility;
    uint8_t *asset;
};

static const struct obstacle_3d_model tower_3d_model = {
    .asset_width = ASSET_TOWER_WIDTH,
    .asset_height = ASSET_TOWER_HEIGHT,
    .z_index_off = TOWER_WIDTH / 2,
    .offset_x = TOWER_OFFSET_X,
    .offset_y = TOWER_OFFSET_Y,
    .default_visibility = false,
    .asset = asset_tower,
};

static const struct obstacle_3d_model grail_holder_3d_model = {
    .asset_width = ASSET_GRAIL_WIDTH,
    .asset_height = ASSET_GRAIL_HEIGHT,
    .z_index_off = GRAIL_WIDTH / 2,
    .offset_x = GRAIL_OFFSET_X,
    .offset_y = GRAIL_OFFSET_Y,
    .default_visibility = true,
    .asset = asset_grail,
};

static const struct obstacle_3d_model pami_3d_model = {
    .asset_width = ASSET_PAMI_WIDTH,
    .asset_height = ASSET_PAMI_HEIGHT,
    .z_index_off = PAMI_WIDTH / 2,
    .offset_x = PAMI_OFFSET_X,
    .offset_y = PAMI_OFFSET_Y,
    .default_visibility = true,
    .asset = asset_pami,
};


sprite_handle_t obstacle_sprites[NUMBER_OF_OBSTACLES];
sprite_handle_t ball_sprites[NUMBER_OF_BALLS];

static void create_block_sprite(uint8_t obstacle_id);
static void create_simple_sprite(uint8_t obstacle_id, const struct obstacle_3d_model *model);
static void create_ball_sprite(uint8_t player_id);
static void game_uv_to_screen_xy(struct point *xy, const struct point *uv);


static struct rect r;
static struct point p;
static uint8_t i;
static sprite_handle_t sprite;
static uint8_t z_index;


void init_view(void)
{
    r.w = ASSET_ROBOT_1E_WIDTH;
    r.h = ASSET_ROBOT_1E_HEIGHT;

    // Create robot sprites
    game_uv_to_screen_xy((struct point*)&r, (struct point*)&robots[0].position);
    r.x -= ROBOT_OFFSET_X;
    r.y -= ROBOT_OFFSET_Y;
    robot_views[0].sprite = create_sprite(asset_robot_1e, &r);
    set_sprite_z_index(robot_views[0].sprite, 0);

    game_uv_to_screen_xy((struct point*)&r, (struct point*)&robots[1].position);
    r.x -= ROBOT_OFFSET_X;
    r.y -= ROBOT_OFFSET_Y;
    robot_views[1].sprite = create_sprite(asset_robot_2w, &r);
    set_sprite_z_index(robot_views[1].sprite, 0);


    // Create obstacle sprites
    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        switch (obstacle_flags[i] & OBSTACLE_FLAG_TYPE) {
            case OBSTACLE_TYPE_QUARRY:
            create_block_sprite(i);
            break;

            case OBSTACLE_TYPE_WALL:
            create_block_sprite(i);
            set_sprite_visibility(sprite, false);
            break;

            case OBSTACLE_TYPE_TOWER:
            create_simple_sprite(i, &tower_3d_model);
            set_sprite_visibility(sprite, false);
            break;

            case OBSTACLE_TYPE_GRAIL_HOLDER:
            create_simple_sprite(i, &grail_holder_3d_model);
            break;

            case OBSTACLE_TYPE_PAMI:
            create_simple_sprite(i, &pami_3d_model);
            break;
        }
    }

    // Balls to throw
    for (i = 0; i < NUMBER_OF_BALLS; ++i) {
        create_ball_sprite(i);
    }

    init_ui();
}


static void create_block_sprite(uint8_t obstacle_id)
{
    p.x = obstacles[obstacle_id].x;
    p.y = obstacles[obstacle_id].y;
    game_uv_to_screen_xy((struct point*)&r, &p);
    z_index = r.y / 2 - (BLOCK_WIDTH + BLOCK_LENGTH) / 4;
    if (IS_ORIENTED_EAST(obstacle_id)) {
        r.x -= EAST_BLOCK_OFFSET_X;
        r.y -= EAST_BLOCK_OFFSET_Y;
    } else {
        r.x -= SOUTH_BLOCK_OFFSET_X;
        r.y -= SOUTH_BLOCK_OFFSET_Y;
    }
    r.w = ASSET_BLOCK_3E_WIDTH;
    r.h = ASSET_BLOCK_3E_HEIGHT;

    if (IS_ORIENTED_EAST(obstacle_id)) {
        sprite = create_sprite(asset_block_3e, &r);
    } else {
        sprite = create_sprite(asset_block_3s, &r);
    }
    obstacle_sprites[obstacle_id] = sprite;
    set_sprite_z_index(sprite, z_index);
    trigger_sprite_redraw(sprite);
}


static void create_simple_sprite(uint8_t obstacle_id, const struct obstacle_3d_model *model)
{
    r.w = model->asset_width;
    r.h = model->asset_height;
    p.x = obstacles[obstacle_id].x;
    p.y = obstacles[obstacle_id].y;
    game_uv_to_screen_xy((struct point*)&r, &p);
    z_index = r.y / 2 - model->z_index_off;
    r.x -= model->offset_x;
    r.y -= model->offset_y;

    sprite = create_sprite(model->asset, &r);
    obstacle_sprites[obstacle_id] = sprite;
    set_sprite_z_index(sprite, z_index);
    set_sprite_visibility(sprite, model->default_visibility);
    trigger_sprite_redraw(sprite);
}


static void create_ball_sprite(uint8_t ball_id)
{
    r.w = ASSET_BLUE_BALL_WIDTH;
    r.h = ASSET_BLUE_BALL_HEIGHT;
    r.x = 0;
    r.y = 0;

    if ((balls[ball_id].flags & BALL_FLAGS_PLAYER_ID_MASK) == BALL_FLAGS_PLAYER_ID_0) {
        sprite = create_sprite(asset_blue_ball, &r);
    } else {
        sprite = create_sprite(asset_yellow_ball, &r);
    }

    ball_sprites[ball_id] = sprite;
    set_sprite_visibility(sprite, false);
}


void update_view(void)
{
    draw_sprites();
    update_ui_display();
}


static void game_uv_to_screen_xy(struct point *xy, const struct point *uv)
{
    xy->x = POSITION_X_ORIGIN + uv->x - uv->y;
    xy->y = POSITION_Y_ORIGIN + uv->x + uv->y;
}


void update_obstacle_sprite(uint8_t obstacle_id)
{
    static bool is_oriented_east;
    static uint8_t stone_quantity;
    static uint8_t obstacle_type;
    static uint8_t player_id;
    static sprite_handle_t grail_sprite;

    is_oriented_east = IS_ORIENTED_EAST(obstacle_id);
    stone_quantity = obstacle_stone_quantity[obstacle_id];
    obstacle_type = obstacle_flags[obstacle_id] & OBSTACLE_FLAG_TYPE;
    sprite = obstacle_sprites[obstacle_id];
    player_id = obstacle_flags[obstacle_id] & OBSTACLE_FLAG_PLAYER_ID;

    if ((obstacle_type == OBSTACLE_TYPE_QUARRY) || (obstacle_type == OBSTACLE_TYPE_WALL)) {
        switch (stone_quantity) {
            case 0:
            set_sprite_visibility(sprite, false);
            break;

            case 1:
            change_sprite_asset(sprite, is_oriented_east ? asset_block_1e : asset_block_1s);
            set_sprite_visibility(sprite, true);
            break;

            case 2:
            change_sprite_asset(sprite, is_oriented_east ? asset_block_2e : asset_block_2s);
            set_sprite_visibility(sprite, true);
            break;

            case 3:
            if (obstacle_type == OBSTACLE_TYPE_QUARRY) {
                change_sprite_asset(sprite, is_oriented_east ? asset_block_3e : asset_block_3s);
            } else {
                change_sprite_asset(sprite, is_oriented_east ? asset_block_3e_built : asset_block_3s_built);
            }
            set_sprite_visibility(sprite, true);
            break;
        }
    } else if (obstacle_type == OBSTACLE_TYPE_TOWER) {
        if (stone_quantity == 0) {
            set_sprite_visibility(sprite, false);
        } else {
            set_sprite_visibility(sprite, true);
        }
    } else if (obstacle_type == OBSTACLE_TYPE_GRAIL_HOLDER) {
        if (grail_locations[obstacle_id - GRAIL_HOLDERS_ID_START] == -1) {
            set_sprite_visibility(sprite, false);
        }
    }

    // The grail is put on top
    if (
        (player_id != OBSTACLE_PLAYER_ID_NONE)
        && (obstacle_id == (uint8_t)grail_locations[player_id])
        && !(obstacle_type == OBSTACLE_TYPE_GRAIL_HOLDER)
    ) {
            grail_sprite = obstacle_sprites[GRAIL_HOLDERS_ID_START + player_id];
            set_sprite_visibility(grail_sprite, true);

            p.x = obstacles[obstacle_id].x;
            p.y = obstacles[obstacle_id].y;
            game_uv_to_screen_xy((struct point*)&r, &p);
            set_sprite_z_index(grail_sprite, r.y / 2 - grail_holder_3d_model.z_index_off + 1);
            r.x -= grail_holder_3d_model.offset_x + GRAIL_DISPLAY_OFFSET_X;
            r.y -= grail_holder_3d_model.offset_y + GRAIL_DISPLAY_OFFSET_Y;
            r.y -= obstacle_stone_quantity[obstacle_id] * GRAIL_DISPLAY_OFFSET_Y_STONE;
            move_sprite(grail_sprite, (struct point*)&r);
            trigger_sprite_redraw(grail_sprite);
    }

    trigger_sprite_redraw(sprite);
}


void update_robot_sprite(uint8_t robot_id, bool change_orientation)
{
    static struct robot_view *view;
    static struct robot_model *robot;

    view = &robot_views[robot_id];
    robot = &robots[robot_id];

    if (change_orientation) {
        change_sprite_asset(view->sprite, view->sprite_assets[robot->orientation]);
    }

    game_uv_to_screen_xy(&p, (struct point*)&robot->position);
    set_sprite_z_index(view->sprite, p.y / 2 - ROBOT_HALF_WIDTH / 2);
    p.x -= ROBOT_OFFSET_X;
    p.y -= ROBOT_OFFSET_Y;
    move_sprite(view->sprite, &p);
    trigger_sprite_redraw(view->sprite);
}


void update_pami_sprite(uint8_t obstacle_id)
{
    sprite = obstacle_sprites[obstacle_id];

    p.x = obstacles[obstacle_id].x;
    p.y = obstacles[obstacle_id].y;
    game_uv_to_screen_xy((struct point*)&r, &p);
    set_sprite_z_index(sprite, p.y / 2 - pami_3d_model.z_index_off);
    p.x -= pami_3d_model.offset_x;
    p.y -= pami_3d_model.offset_y;

    move_sprite(sprite, (struct point*)&r);
    trigger_sprite_redraw(sprite);
}


void update_ball_sprite(uint8_t ball_id)
{
    sprite = ball_sprites[ball_id];

    p.x = balls[ball_id].position.x;
    p.y = balls[ball_id].position.y;
    game_uv_to_screen_xy((struct point*)&r, &p);
    set_sprite_z_index(sprite, r.y / 2);
    r.y -= ball_parabola[balls[ball_id].parabolic_index] >> balls[ball_id].damping_factor;

    set_sprite_visibility(sprite, true);

    move_sprite(sprite, (struct point*)&r);
    trigger_sprite_redraw(sprite);
}


void clear_view(void)
{
    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        set_sprite_visibility(obstacle_sprites[i], false);
        trigger_sprite_redraw(obstacle_sprites[i]);
    }
    set_sprite_visibility(robot_views[0].sprite, false);
    set_sprite_visibility(robot_views[1].sprite, false);

    for (i = 0; i < NUMBER_OF_BALLS; ++i) {
        set_sprite_visibility(ball_sprites[i], false);
    }

    swap_buffers();
    draw_sprites();
    swap_buffers();
    draw_sprites();

    clear_ui_elements();

    set_double_buffering(false);
}
