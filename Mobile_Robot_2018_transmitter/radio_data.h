#pragma once
#include "Arduino.h"

struct radioDataTrasnsmit {
  byte analog_left_X,
       analog_left_Y,
       analog_right_X,
       analog_right_Y,
       reserved0,
       reserved1,
       reserved2,
       potentiometer,
       control_mode,
       reserved3,
       bit_array,
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