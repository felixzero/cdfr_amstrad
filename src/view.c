#include "view.h"

#include "sprites.h"
#include "inputs.h"
#include "sprite_assets.h"
#include "ui.h"
#include "controller.h"

#include <string.h>

struct robot_view {
    sprite_handle_t sprite;
    uint8_t *sprite_assets[4];
};

struct robot_view robot_views[2] = {
    { .sprite = 0, .sprite_assets = { asset_robot_1e, asset_robot_1s, asset_robot_1w, asset_robot_1n }},
    { .sprite = 0, .sprite_assets = { asset_robot_2e, asset_robot_2s, asset_robot_2w, asset_robot_2n }},
};

sprite_handle_t obstacle_sprites[NUMBER_OF_OBSTACLES];

static void create_quarry_sprite(uint8_t obstacle_id);
static void create_wall_sprite(uint8_t obstacle_id);
static void create_tower_sprite(uint8_t obstacle_id);
static void create_grail_sprite(uint8_t obstacle_id);
static void create_pami_sprite(uint8_t obstacle_id);
static inline void game_uv_to_screen_xy(struct point *xy, const struct point *uv);


static struct rect r;
static struct point p;
static uint8_t i;
static sprite_handle_t sprite;


void init_view(void)
{
    // Create robot sprites
    r.w = ASSET_ROBOT_1E_WIDTH;
    r.h = ASSET_ROBOT_1E_HEIGHT;
    game_uv_to_screen_xy(&p, &robots[0].position);
    r.x = p.x;
    r.y = p.y;
    robot_views[0].sprite = create_sprite(asset_robot_1e, &r);
    set_sprite_z_index(robot_views[0].sprite, 0);
    game_uv_to_screen_xy(&p, &robots[1].position);
    r.x = p.x;
    r.y = p.y;
    robot_views[1].sprite = create_sprite(asset_robot_2w, &r);
    set_sprite_z_index(robot_views[1].sprite, 0);


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

            case OBSTACLE_TYPE_GRAIL_HOLDER:
            create_grail_sprite(i);
            break;

            case OBSTACLE_TYPE_PAMI:
            create_pami_sprite(i);
            break;
        }
    }

    init_ui();
}


static void create_quarry_sprite(uint8_t obstacle_id)
{
    r.w = ASSET_BLOCK_3E_WIDTH;
    r.h = ASSET_BLOCK_3E_HEIGHT;
    p.x = obstacles[obstacle_id].x + QUARRY_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + QUARRY_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    if (IS_ORIENTED_EAST(obstacle_id)) {
        sprite = create_sprite(asset_block_3e, &r);
    } else {
        sprite = create_sprite(asset_block_3s, &r);
    }
    obstacle_sprites[obstacle_id] = sprite;
    set_sprite_z_index(sprite, r.y / 2 - QUARRY_Z_INDEX_OFFSET / 2);
    trigger_sprite_redraw(sprite);

    obstacle_stone_quantity[obstacle_id] = INITIAL_NUMBER_OF_STONES;
}


static void create_wall_sprite(uint8_t obstacle_id)
{
    r.w = ASSET_BLOCK_3E_WIDTH;
    r.h = ASSET_BLOCK_3E_HEIGHT;
    p.x = obstacles[obstacle_id].x + WALL_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + WALL_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    if (IS_ORIENTED_EAST(obstacle_id)) {
        sprite = create_sprite(asset_block_1e, &r);
    } else {
        sprite = create_sprite(asset_block_1s, &r);
    }
    obstacle_sprites[obstacle_id] = sprite;
    set_sprite_z_index(sprite, r.y / 2 - WALL_Z_INDEX_OFFSET / 2);
    set_sprite_visibility(sprite, false);
    trigger_sprite_redraw(sprite);

    obstacle_stone_quantity[obstacle_id] = 0;
}


static void create_tower_sprite(uint8_t obstacle_id)
{
    r.w = ASSET_TOWER_WIDTH;
    r.h = ASSET_TOWER_HEIGHT;
    p.x = obstacles[obstacle_id].x + TOWER_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + TOWER_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    sprite = create_sprite(asset_tower, &r);
    obstacle_sprites[obstacle_id] = sprite;
    set_sprite_z_index(sprite, r.y / 2 - WALL_Z_INDEX_OFFSET / 2);
    set_sprite_visibility(sprite, false);
    trigger_sprite_redraw(sprite);

    obstacle_stone_quantity[obstacle_id] = 0;
}


static void create_grail_sprite(uint8_t obstacle_id)
{
    r.w = ASSET_GRAIL_WIDTH;
    r.h = ASSET_GRAIL_HEIGHT;
    p.x = obstacles[obstacle_id].x + GRAIL_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + GRAIL_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    sprite = create_sprite(asset_grail, &r);
    obstacle_sprites[obstacle_id] = sprite;
    set_sprite_z_index(sprite, r.y / 2 - GRAIL_Z_INDEX_OFFSET / 2);
    trigger_sprite_redraw(sprite);
}


static void create_pami_sprite(uint8_t obstacle_id)
{
    r.w = ASSET_PAMI_WIDTH;
    r.h = ASSET_PAMI_HEIGHT;
    p.x = obstacles[obstacle_id].x + PAMI_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + PAMI_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

    sprite = create_sprite(asset_pami, &r);
    obstacle_sprites[obstacle_id] = sprite;
    set_sprite_z_index(sprite, r.y / 2 - PAMI_Z_INDEX_OFFSET / 2);
    trigger_sprite_redraw(sprite);
}


void update_view(void)
{
    draw_sprites();
    update_ui_display();
}


static inline void game_uv_to_screen_xy(struct point *xy, const struct point *uv)
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

            p.x = obstacles[obstacle_id].x + GRAIL_DISPLAY_OFFSET_U;
            p.y = obstacles[obstacle_id].y + GRAIL_DISPLAY_OFFSET_V;
            game_uv_to_screen_xy((struct point*)&r, &p);
            r.y -= obstacle_stone_quantity[obstacle_id] * GRAIL_DISPLAY_OFFSET_Y_STONE;
            move_sprite(grail_sprite, (struct point*)&r);
            set_sprite_z_index(grail_sprite, r.y / 2 + 2);
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

    game_uv_to_screen_xy(&p, &robot->position);
    move_sprite(view->sprite, &p);
    set_sprite_z_index(view->sprite, p.y / 2);
    trigger_sprite_redraw(view->sprite);
}


void update_pami_sprite(uint8_t obstacle_id)
{
    sprite = obstacle_sprites[obstacle_id];

    p.x = obstacles[obstacle_id].x + PAMI_DISPLAY_OFFSET_U;
    p.y = obstacles[obstacle_id].y + PAMI_DISPLAY_OFFSET_V;
    game_uv_to_screen_xy((struct point*)&r, &p);

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

    swap_buffers();
    draw_sprites();
    swap_buffers();
    draw_sprites();

    clear_ui_elements();

    set_double_buffering(false);
}
