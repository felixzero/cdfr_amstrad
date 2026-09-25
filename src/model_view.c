#include "model_view.h"

//#include "game_model.h"
#define NUMBER_OF_QUARRIES  10
#include "sprites.h"
#include "inputs.h"
#include "sprite_assets.h"

#define ORIENTATION_EAST    0
#define ORIENTATION_SOUTH   1
#define ORIENTATION_WEST    2
#define ORIENTATION_NORTH   3

#define CONTROL_KEY_UP      0
#define CONTROL_KEY_DOWN    1
#define CONTROL_KEY_RIGHT   2
#define CONTROL_KEY_LEFT    3

#define ROTATION_TIMER_MASK    0x0F

static const uint8_t quarry_x_positions[] = { 68, 90, 10, 32, 138, 116, 80, 58, 86, 60 };
static const uint8_t quarry_y_positions[] = { 22, 44, 80, 102, 92, 70, 150, 128, 73, 100 };
static sprite_handle_t quarry_sprites[NUMBER_OF_QUARRIES];

struct robot_model {
    struct point position;
    uint8_t orientation;
    uint8_t carried_stones;
    uint8_t rotation_timer;

    sprite_handle_t sprite;
    uint8_t *sprite_assets[4];
    uint32_t control_keys[4];
};


struct robot_model robots[2];

static inline void game_uv_to_screen_xy(struct point *xy, const struct point *uv);

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

    robots[1].sprite_assets[ORIENTATION_EAST] = asset_robot_2e;
    robots[1].sprite_assets[ORIENTATION_SOUTH] = asset_robot_2s;
    robots[1].sprite_assets[ORIENTATION_WEST] = asset_robot_2w;
    robots[1].sprite_assets[ORIENTATION_NORTH] = asset_robot_2n;
    robots[1].control_keys[CONTROL_KEY_UP] = KEY_UP;
    robots[1].control_keys[CONTROL_KEY_DOWN] = KEY_DOWN;
    robots[1].control_keys[CONTROL_KEY_RIGHT] = KEY_RIGHT;
    robots[1].control_keys[CONTROL_KEY_LEFT] = KEY_LEFT;

    robots[0].position.x = 0;
    robots[0].position.y = 0;
    robots[0].orientation = ORIENTATION_EAST;

    robots[1].position.x = 50;
    robots[1].position.y = 0;
    robots[1].orientation = ORIENTATION_WEST;

    struct point p;
    struct rect r;
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

    r.w = ASSET_BLOCK_3E_WIDTH;
    r.h = ASSET_BLOCK_3E_HEIGHT;
    for (uint8_t i = 0; i < NUMBER_OF_QUARRIES - 2; ++i) {
        r.x = quarry_x_positions[i];
        r.y = quarry_y_positions[i];
        quarry_sprites[i] = create_sprite(asset_block_3e, &r);
        set_sprite_z_index(quarry_sprites[i], 255 - r.y);
    }
    for (uint8_t i = NUMBER_OF_QUARRIES - 2; i < NUMBER_OF_QUARRIES; ++i) {
        r.x = quarry_x_positions[i];
        r.y = quarry_y_positions[i];
        quarry_sprites[i] = create_sprite(asset_block_3s, &r);
        set_sprite_z_index(quarry_sprites[i], 255 - r.y);
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
            if (robots[i].rotation_timer == 0) {
                robots[i].orientation++;
                robots[i].orientation &= 0x03;
                change_sprite_asset(robots[i].sprite, robots[i].sprite_assets[robots[i].orientation]);
            }
            robots[i].rotation_timer++;
            robots[i].rotation_timer &= ROTATION_TIMER_MASK;
        } else if (!(keys & robots[i].control_keys[CONTROL_KEY_LEFT])) {
            if (robots[i].rotation_timer == 0) {
                robots[i].orientation--;
                robots[i].orientation &= 0x03;
                change_sprite_asset(robots[i].sprite, robots[i].sprite_assets[robots[i].orientation]);
            }
            robots[i].rotation_timer++;
            robots[i].rotation_timer &= ROTATION_TIMER_MASK;
        } else if (!(keys & robots[i].control_keys[CONTROL_KEY_UP]) || !(keys & robots[i].control_keys[CONTROL_KEY_DOWN])) {
            robots[i].rotation_timer = 0;
            increment = !(keys & robots[i].control_keys[CONTROL_KEY_UP]) ? 2 : -2;
            switch (robots[i].orientation) {
                case ORIENTATION_EAST:
                robots[i].position.x += increment;
                break;

                case ORIENTATION_WEST:
                robots[i].position.x -= increment;
                break;

                case ORIENTATION_SOUTH:
                robots[i].position.y += increment;
                break;

                case ORIENTATION_NORTH:
                robots[i].position.y -= increment;
                break;
            }
        } else {
            robots[i].rotation_timer = 0;
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
    xy->x = 58 + uv->x - uv->y;
    xy->y = 8 + uv->x + uv->y;
}
