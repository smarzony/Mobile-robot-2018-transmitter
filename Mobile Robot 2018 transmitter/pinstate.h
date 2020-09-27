#pragma once
#include <stdint.h>

struct PinState {
  uint8_t no;
  bool actual = false;
  bool was_pushed = false;
  bool last = false;
  bool action_running = false;
  int hold_periods = 0;
  bool rising();
  bool falling();
  bool change();
  unsigned long long time_save;
};


