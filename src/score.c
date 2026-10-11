#include "score.h"
#include "graphics.h"
#include "print.h"
#include "model.h"
#include "view.h"

#include <stdint.h>

#define PALETTE_BLACK 1
#define PALETTE_WHITE 15
#define PALETTE_BLUE 3
#define PALETTE_YELLOW 13

#define Y_ELEMENT 3
#define Y_BLUE 13
#define Y_YELLOW 18

#define POINT_PER_STONE 1
#define POINT_PER_TOWER 3
#define POINT_PER_WALL 2
#define POINT_PER_GATE 5
#define POINT_PER_PAMI 5
#define POINT_PER_GRAIL 15
#define POINT_FULL_RETURN 10
#define POINT_PARTIAL_RETURN 5

static void display_score_line(uint8_t line, const char* label, uint8_t blue_score, uint8_t yellow_score);
static uint8_t calculate_stone_score(uint8_t player_id);
static uint8_t calculate_wall_score(uint8_t player_id);
static uint8_t calculate_tower_score(uint8_t player_id);
static uint8_t calculate_return_score(uint8_t player_id);

static uint8_t i, score;

void display_scores(void)
{
    static uint8_t total_0, total_1, score_0, score_1;

    total_0 = 0;
    total_1 = 0;

    clear_view();

    set_text_palette(PALETTE_WHITE, PALETTE_BLUE);
    move_cursor(Y_BLUE, 3);
    prints("Bleu");
    set_text_palette(PALETTE_BLACK, PALETTE_YELLOW);
    move_cursor(Y_YELLOW, 3);
    prints("Jaune");

    score_0 = calculate_stone_score(0);
    score_1 = calculate_stone_score(1);
    total_0 += score_0;
    total_1 += score_1;
    display_score_line(5,  "Pierres:", score_0, score_1);

    score_0 = calculate_wall_score(0);
    score_1 = calculate_wall_score(1);
    total_0 += score_0;
    total_1 += score_1;
    display_score_line(6,  "Murs:   ", score_0, score_1);

    score_0 = calculate_tower_score(0);
    score_1 = calculate_tower_score(1);
    total_0 += score_0;
    total_1 += score_1;
    display_score_line(7,  "Tours:  ", score_0, score_1);

    score_0 = (grail_locations[0] == GRAIL_HOLDERS_ID_START) || (grail_locations[0] == -1) ? 0 : POINT_PER_GRAIL;
    score_1 = (grail_locations[1] == GRAIL_HOLDERS_ID_START + 1) || (grail_locations[1] == -1) ? 0 : POINT_PER_GRAIL;
    total_0 += score_0;
    total_1 += score_1;
    display_score_line(9,  "Graal:  ", score_0, score_1);

    score_0 = calculate_return_score(0);
    score_1 = calculate_return_score(1);
    total_0 += score_0;
    total_1 += score_1;
    display_score_line(11, "Retour: ", score_0, score_1);

    score_0 = pami_stuck[0] ? 0 : POINT_PER_PAMI;
    score_1 = pami_stuck[1] ? 0 : POINT_PER_PAMI;
    total_0 += score_0;
    total_1 += score_1;
    display_score_line(13, "PAMI:   ", score_0, score_1);

    display_score_line(14, "Boulet: ", 0, 0);

    display_score_line(16, "Total:  ", total_0, total_1);

    if (total_0 > total_1) {
        set_text_palette(PALETTE_WHITE, PALETTE_BLACK);
        move_cursor(3, 18);
        prints("Victoire de");
        move_cursor(15, 18);
        set_text_palette(PALETTE_WHITE, PALETTE_BLUE);
        prints("bleu");
    } else if (total_1 > total_0) {
        set_text_palette(PALETTE_WHITE, PALETTE_BLACK);
        move_cursor(3, 18);
        prints("Victoire de");
        move_cursor(15, 18);
        set_text_palette(PALETTE_BLACK, PALETTE_YELLOW);
        prints("jaune");
    } else {
        set_text_palette(PALETTE_WHITE, PALETTE_BLACK);
        move_cursor(3, 18);
        prints("Match nul");
    }

    move_cursor(0, 19);
    prints("Appuyez sur A pour rejouer");
}


static void display_score_line(uint8_t line, const char* label, uint8_t blue_score, uint8_t yellow_score)
{
    set_text_palette(PALETTE_WHITE, PALETTE_BLACK);
    move_cursor(Y_ELEMENT, line);
    prints(label);

    set_text_palette(PALETTE_WHITE, PALETTE_BLUE);
    move_cursor(Y_BLUE, line);
    printint(blue_score);

    set_text_palette(PALETTE_BLACK, PALETTE_YELLOW);
    move_cursor(Y_YELLOW, line);
    printint(yellow_score);  
}


static uint8_t calculate_stone_score(uint8_t player_id)
{
    score = 0;
    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        if ((obstacle_flags[i] & OBSTACLE_FLAG_PLAYER_ID) != player_id) {
            continue;
        }
        score += POINT_PER_STONE * obstacle_stone_quantity[i];
    }

    return score;
}


static uint8_t calculate_wall_score(uint8_t player_id)
{
    score = 0;
    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        if ((obstacle_flags[i] & OBSTACLE_FLAG_PLAYER_ID) != player_id) {
            continue;
        }

        if ((obstacle_flags[i] & OBSTACLE_FLAG_TYPE) == OBSTACLE_TYPE_WALL && (obstacle_stone_quantity[i] == 3)) {
            score += POINT_PER_WALL;
        }
    }

    return score;
}


static uint8_t calculate_tower_score(uint8_t player_id)
{
    score = 0;
    for (i = 0; i < NUMBER_OF_OBSTACLES; ++i) {
        if (
            ((obstacle_flags[i] & OBSTACLE_FLAG_PLAYER_ID) == player_id)
            && ((obstacle_flags[i] & OBSTACLE_FLAG_TYPE) == OBSTACLE_TYPE_TOWER)
            && (obstacle_stone_quantity[i] > 0)
        ) {
            score += POINT_PER_TOWER;
        }
    }

    return score;
}

#define FULL_RETURN_MARGIN 6
#define PARTIAL_RETURN_MARGIN 12

const struct rect full_return_box[] = {
    {
        .x = 0,
        .y = ROBOT_STARTING_POSITION_V - FULL_RETURN_MARGIN / 2,
        .w = FULL_RETURN_MARGIN,
        .h = FULL_RETURN_MARGIN
    },
    {
        .x = TABLE_EDGE_U - FULL_RETURN_MARGIN / 2,
        .y = ROBOT_STARTING_POSITION_V - FULL_RETURN_MARGIN / 2,
        .w = FULL_RETURN_MARGIN,
        .h = FULL_RETURN_MARGIN
    },
};
const struct rect partial_return_box[] = {
    {
        .x = 0,
        .y = ROBOT_STARTING_POSITION_V - PARTIAL_RETURN_MARGIN / 2,
        .w = PARTIAL_RETURN_MARGIN,
        .h = PARTIAL_RETURN_MARGIN
    },
    {
        .x = TABLE_EDGE_U - PARTIAL_RETURN_MARGIN / 2,
        .y = ROBOT_STARTING_POSITION_V - PARTIAL_RETURN_MARGIN / 2,
        .w = PARTIAL_RETURN_MARGIN,
        .h = PARTIAL_RETURN_MARGIN
    },
};

static uint8_t calculate_return_score(uint8_t player_id)
{
    struct point *robot_position = &robots[player_id].position;

    if (rect_contains(&full_return_box[player_id], robot_position)) {
        return POINT_FULL_RETURN;
    } else if (rect_contains(&partial_return_box[player_id], robot_position)) {
        return POINT_PARTIAL_RETURN;
    }

    return 0;
}
