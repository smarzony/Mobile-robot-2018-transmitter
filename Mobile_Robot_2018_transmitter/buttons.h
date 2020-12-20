#pragma once
#include "Arduino.h"
#include <stdint.h>
#include "pinstate.h"
#include "jm_PCF8574.h"
#include "timer.h"

#define BUTTON_DELAY 300

void read_button_neg_switch(uint8_t button, bool &state, jm_PCF8574 &remoteIO);
void read_button_inc_switch(uint8_t button, uint8_t limit_min, uint8_t limit_max, uint8_t& state, jm_PCF8574 &remoteIO);
void read_button_dec_switch(uint8_t button, uint8_t limit_min, uint8_t limit_max, uint8_t& state, jm_PCF8574 &remoteIO);
void read_button_inc_dec_switch(uint8_t button, uint8_t button1, uint8_t limit_min, uint8_t limit_max, uint8_t& state, jm_PCF8574 &remoteIO);
void button_hold(PinState &pin, int &input_value, void (*operation)(int, int&));