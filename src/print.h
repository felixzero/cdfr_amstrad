#pragma once

#include <stdint.h>

void move_cursor(uint8_t col, uint8_t line);

void set_text_palette(uint8_t foreground, uint8_t background);

void printc(char c);

void prints(const char *s);

void printint(uint8_t v);
