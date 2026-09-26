#include "model_view.h"

#include "sprites.h"
#include "inputs.h"
#include "sprite_assets.h"

#define ORIENTATION_EAST                0
#define ORIENTATION_SOUTH               1
#define ORIENTATION_WEST                2
#define ORIENTATION_NORTH               3

#define CONTROL_KEY_UP                  0
#define CONTROL_KEY_DOWN                1
#define CONTROL_KEY_RIGHT               2
#define CONTROL_KEY_LEFT                3
#define CONTROL_KEY_ACTION              4

#define POSITION_X_ORIGIN               58
#define POSITION_Y_ORIGIN               8

#define TABLE_EDGE_U                    90
#define TABLE_EDGE_V                    58

#define ROBOT_STARTING_POSITION_V       30
#define ROBOT_HITBOX                    8
#define ROBOT_INTERACTION_RADIUS        4
#define ROTATION_TIMER_MASK             0x0F
#define MINING_TIMER_OUT                16
#define ROBOT_MINING_DISTANCE           4

#define NUMBER_OF_QUARRIES              10
#define QUARRY_DISPLAY_OFFSET_U         8
#define QUARRY_DISPLAY_OFFSET_V         7
#define QUARRY_Z_INDEX_OFFSET           8
#define QUARRY_SOUTH_ORIENTATION_INDEX  (NUMBER_OF_QUARRIES - 2)
#define IS_QUARRY_ORIENTED_EAST(x)      ((x) < QUARRY_SOUTH_ORIENTATION_INDEX)
#define QUARRY_HITBOX_U                 14
#define QUARRY_HITBOX_V                 10
#define INITIAL_NUMBER_OF_STONES        3

#define COLLISION_OUT_OF_TABLE          -1
#define COLLISION_OTHER_ROBOT           -2
#define COLLISION_NOTHING               0
#define COLLISION_QUARRY_START          1

static const int8_t quarry_u_positions[] = {  4, 26,  4,  26, 74, 52, 74, 52, 38, 39 };
static const int8_t quarry_v_positions[] = { -5, -5, 53,  53, -5, -5, 53, 53, 11, 38 };
static sprite_handle_t quarry_sprites[NUMBER_OF_QUARRIES];

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
static uint8_t quarry_stone_quantity[NUMBER_OF_QUARRIES];

static inline void game_uv_to_screen_xy(struct point *xy, const struct point *uv);
static int8_t check_collisions(struct point *uv);
static void displace_robot(struct point *destination, uint8_t robot_id, int8_t increment);
static void pick_stone_from_quarry(uint8_t quarry_id);


void init_model_view(void)
{
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

    struct point p;
    struct rect r;

    // Create robot sprites
    r.w = ASSET_ROBOT_1E_WIDTH;
    r.h = ASSET_ROBOT_1E_HEIGHT;
    game_uv_to_screen_xy(&p, &robots[0].position);
    r.x = p.x;
    r.y = p.y;
    robots[0].sprite = create_sprite(asset_robot_1e, &r);
    set_sprite_z_index(robots[0].sprite, 200);
    game_uv_to_screen_xy(&p, &robots[1].position);
    r.x = p.x;
    r.y = p.y;
    robots[1].sprite = create_sprite(asset_robot_2w, &r);
    set_sprite_z_index(robots[1].sprite, 200);

    // Create quarry sprites
    r.w = ASSET_BLOCK_3E_WIDTH;
    r.h = ASSET_BLOCK_3E_HEIGHT;
    for (uint8_t i = 0; i < NUMBER_OF_QUARRIES; ++i) {
        p.x = quarry_u_positions[i] + QUARRY_DISPLAY_OFFSET_U;
        p.y = quarry_v_positions[i] + QUARRY_DISPLAY_OFFSET_V;
        game_uv_to_screen_xy((struct point*)&r, &p);
        if (IS_QUARRY_ORIENTED_EAST(i)) {
            quarry_sprites[i] = create_sprite(asset_block_3e, &r);
        } else {
            quarry_sprites[i] = create_sprite(asset_block_3s, &r);
        }
        set_sprite_z_index(quarry_sprites[i], 255 - r.y + QUARRY_Z_INDEX_OFFSET);
        trigger_sprite_redraw(quarry_sprites[i]);

        quarry_stone_quantity[i] = INITIAL_NUMBER_OF_STONES;
    }
}


void update_graphics(void)
{
    static uint32_t keys;
    static uint8_t increment;
    static uint8_t i;
    static struct point p;

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

            if (!check_collisions(&p)) {
                robots[i].position.x = p.x;
                robots[i].position.y = p.y;
            }
        } else if (!(keys & robots[i].control_keys[CONTROL_KEY_ACTION])) {
            displace_robot(&p, i, ROBOT_MINING_DISTANCE);
            int8_t quarry_id = check_collisions(&p);
            if (quarry_id >= COLLISION_QUARRY_START) {
                quarry_id -= COLLISION_QUARRY_START;
                if ((robots[i].action_timer == MINING_TIMER_OUT) && (quarry_stone_quantity[quarry_id] > 0)) {
                    pick_stone_from_quarry(quarry_id);
                    robots[i].carried_stones++;
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
        set_sprite_z_index(robots[i].sprite, 255 - p.y);
        trigger_sprite_redraw(robots[i].sprite); // 6.5 ms

    }

    draw_sprites(); // 14 ms, including 5.5 ms outside blits

    wait_for_vsync();
    swap_buffers();
}


static inline void game_uv_to_screen_xy(struct point *xy, const struct point *uv)
{
    xy->x = POSITION_X_ORIGIN + uv->x - uv->y;
    xy->y = POSITION_Y_ORIGIN + uv->x + uv->y;
}


static int8_t check_collisions(struct point *uv)
{
    static struct rect r;
    static uint8_t i;

    r.w = ROBOT_HITBOX * 2;
    r.h = ROBOT_HITBOX * 2;
    r.x = robots[1].position.x - ROBOT_HITBOX;
    r.y = robots[1].position.y - ROBOT_HITBOX;

    // Out of table
    if ((uv->x & 0x80) || (uv->y & 0x80) || (uv->x > TABLE_EDGE_U) || (uv->y > TABLE_EDGE_V)) {
        return COLLISION_OUT_OF_TABLE;
    }

    // Robot collision
    if (rect_contains(&r, &robots[0].position)) {
        return COLLISION_OTHER_ROBOT;
    }

    // Quarry collision
    for (i = 0; i < NUMBER_OF_QUARRIES; ++i) {
        if (IS_QUARRY_ORIENTED_EAST(i)) {
            r.w = QUARRY_HITBOX_U;
            r.h = QUARRY_HITBOX_V;
        } else {
            r.w = QUARRY_HITBOX_V;
            r.h = QUARRY_HITBOX_U;
        }
        
        r.x = quarry_u_positions[i];
        r.y = quarry_v_positions[i];

        if (rect_contains(&r, uv)) {
            return COLLISION_QUARRY_START + i;
        }
    }

    return COLLISION_NOTHING;
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

static void pick_stone_from_quarry(uint8_t quarry_id)
{
    quarry_stone_quantity[quarry_id]--;
    
    switch (quarry_stone_quantity[quarry_id]) {
        case 0:
        set_sprite_z_index(quarry_sprites[quarry_id], SPRITE_Z_INDEX_HIDDEN);
        break;

        case 1:
        change_sprite_asset(quarry_sprites[quarry_id], asset_block_1e);
        break;

        case 2:
        change_sprite_asset(quarry_sprites[quarry_id], asset_block_2e);
        break;

        case 3:
        change_sprite_asset(quarry_sprites[quarry_id], asset_block_3e);
        break;
    }
    
    trigger_sprite_redraw(quarry_sprites[quarry_id]);
}
