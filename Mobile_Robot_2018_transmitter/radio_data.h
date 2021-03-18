#pragma once
#include "Arduino.h"

struct radioDataTrasnsmit {
  byte analog_left_X,
       analog_left_Y,
       analog_right_X,
       analog_right_Y,
       servo_0,
       led_g,
       led_b,
       potentiometer,
       control_mode,
       rotory_encoder,
       bit_array,
       //bit_array2,
       message_no;
};

struct radioDataReceive {
  byte velocity_measured_left,
       velocity_measured_right,
       distance,
       control_mode,
       time_delay,
       reserved5,
       reserved6,
       reserved7,
       reserved8,
       message_no;
};