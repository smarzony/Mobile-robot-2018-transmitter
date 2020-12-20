#include "Arduino.h"

// #define ANALOG_LEFT_X 2
// #define ANALOG_LEFT_Y 3
// #define ANALOG_RIGHT_X 0
// #define ANALOG_RIGHT_Y 1
// #define NODEMCU

uint8_t byte_limit(int value)
{
  if (value > 255)
    value = 255;
  if (value < 0)
    value = 0;
  return uint8_t(value);
}


