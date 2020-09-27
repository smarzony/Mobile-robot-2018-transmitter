#pragma once
#include "Arduino.h"

struct analogCorrection {
  int analog_left_X_correct = 0;
  int analog_left_Y_correct = 0;
  int analog_right_X_correct = 0;
  int analog_right_Y_correct = 0;
};