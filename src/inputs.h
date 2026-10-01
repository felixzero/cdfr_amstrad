#pragma once

#include <stdint.h>

// Read device
void get_keypress(void);

// Should only be used as arguments for is_key_pressed
void check_key_up();
void check_key_down();
void check_key_left();
void check_key_right();
void check_key_action();

typedef void (*key_press_function)();

// Returns true if the key for the specific function is pressed for player_id
bool is_key_pressed(uint8_t player_id, key_press_function function);
