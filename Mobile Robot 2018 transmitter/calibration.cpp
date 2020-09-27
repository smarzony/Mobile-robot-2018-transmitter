#include "calibration.h"

void calibration(analogCorrection &analog_correction)
{
  #ifdef PROMINI
    analog_correction.analog_left_X_correct = 128 - (analogRead(ANALOG_LEFT_X) / 4);
    analog_correction.analog_left_Y_correct = 128 - (analogRead(ANALOG_LEFT_Y) / 4);
    analog_correction.analog_right_X_correct = 128 - (analogRead(ANALOG_RIGHT_X) / 4);
    analog_correction.analog_right_Y_correct = 128 - (analogRead(ANALOG_RIGHT_Y) / 4);
  #endif

  #ifdef NODEMCU
    analog_correction.analog_left_X_correct = 128 - (remoteAI.readADC_SingleEnded(ANALOG_LEFT_X ) / 4);
    analog_correction.analog_left_Y_correct = 128 - (remoteAI.readADC_SingleEnded(ANALOG_LEFT_Y )/ 4);
    analog_correction.analog_right_X_correct = 128 - (remoteAI.readADC_SingleEnded(ANALOG_RIGHT_X ) / 4);
    analog_correction.analog_right_Y_correct = 128 - (remoteAI.readADC_SingleEnded(ANALOG_RIGHT_Y ) / 4);
  #endif

  #ifdef NODEMCU
    save_memory(0, 1, analog_correction.analog_left_X_correct + 128);
    save_memory(1, 1, analog_correction.analog_left_Y_correct + 128);
    save_memory(2, 1, analog_correction.analog_right_X_correct + 128);
    save_memory(3, 1, analog_correction.analog_right_Y_correct + 128);
  #endif
}
