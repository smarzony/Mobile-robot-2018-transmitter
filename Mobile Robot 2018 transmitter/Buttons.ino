
void tactileSwitchesHandler()
{
  read_button_neg_switch(ANALOG_LEFT_PUSHBUTTON, analog_left_switch_state);
  read_button_neg_switch(ANALOG_RIGHT_PUSHBUTTON, analog_right_switch_state);
  read_button_inc_switch(ROTORY_ENCODER_PUSHBUTTON, 0, ROTORY_ENCODER_SWITCH_MAX, rotory_encoder.switch_value);

  //	if (digitalRead(SIDE_SWITCH) == 0)
  //		read_button_inc_switch(ROTORY_ENCODER_PUSHBUTTON, 0, ROTORY_ENCODER_SWITCH_MAX, rotory_encoder.switch_value);
  //	else
  read_button_inc_switch(ROTORY_ENCODER_PUSHBUTTON, 0, 7, rotory_encoder.switch_value);

}

void read_button_neg_switch(byte button, bool &state)
{
  static unsigned long lastTime;
  unsigned long timeNow = millis();
  #ifdef PROMINI
  if (digitalRead(button) == 0) {
    if (timeNow - lastTime < BUTTON_DELAY)
      return;
    if (digitalRead(button) == 0)
    {
      state = !state;
    }
    lastTime = timeNow;
  }
  #endif

  #ifdef NODEMCU
  if (remoteIO.digitalRead(button) == 0) {
    if (timeNow - lastTime < BUTTON_DELAY)
      return;
    if (remoteIO.digitalRead(button) == 0)
    {
      state = !state;
    }
    lastTime = timeNow;
  }
  #endif
}

void read_button_inc_switch(byte button, int limit_min, int limit_max, uint8_t& state)
{
  static unsigned long lastTime;
  unsigned long timeNow = millis();
  #ifdef PROMINI
  if (digitalRead(button) == 0) {
    if (timeNow - lastTime < BUTTON_DELAY)
      return;
    if (digitalRead(button) == 0)
    {
      state += 1;
      if (state <= limit_max)
      {
        //        Serial.print("Inc switch: ");
        //        Serial.println(state);
      }
    }
    lastTime = timeNow;
  }
  #endif

  #ifdef NODEMCU
  if (remoteIO.digitalRead(button) == 0) {
    if (timeNow - lastTime < BUTTON_DELAY)
      return;
    if (remoteIO.digitalRead(button) == 0)
    {
      state += 1;
      if (state <= limit_max)
      {
        //        Serial.print("Inc switch: ");
        //        Serial.println(state);
      }
    }
    lastTime = timeNow;
  }
  #endif
 
  if (state > limit_max)
  {
    state = limit_min;
  }
  if (state < limit_min)
  {
    state = limit_max;
  }

}

void calibration()
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


	save_memory(0, 1, analog_correction.analog_left_X_correct + 128);
	save_memory(1, 1, analog_correction.analog_left_Y_correct + 128);
	save_memory(2, 1, analog_correction.analog_right_X_correct + 128);
	save_memory(3, 1, analog_correction.analog_right_Y_correct + 128);
}
