#pragma once
#include <stdint.h>
#include "pinstate.h"

#define BUTTON_DELAY 300

void read_button_neg_switch(uint8_t button, bool &state, int (*read_button_callback)(uint8_t));
void read_button_inc_switch(uint8_t button, uint8_t limit_min, uint8_t limit_max, uint8_t& state, int (*read_button_callback)(uint8_t));
void button_hold(PinState &pin, uint8_t &input_value, void (*operation)(int, uint8_t&));