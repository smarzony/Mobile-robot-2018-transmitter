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
  uint8_t velocity_measured_left,
       velocity_measured_right,
       distance,
       control_mode,
       time_delay,
       azimuth1,
       azimuth2,
       message_no;
  uint8_t pos_lat[4];
  uint8_t pos_long[4];       
};