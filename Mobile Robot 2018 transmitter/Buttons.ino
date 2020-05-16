void tactileSwitchesHandler()
{
	read_button_neg_switch(ANALOG_LEFT_PUSHBUTTON, analog_left_switch_state);
	read_button_neg_switch(ANALOG_RIGHT_PUSHBUTTON, analog_right_switch_state);
	read_button_inc_switch(ROTORY_ENCODER_PUSHBUTTON, 0, ROTORY_ENCODER_SWITCH_MAX, rotory_encoder.switch_value);
	/*
	if (digitalRead(SIDE_SWITCH) == 0)
		read_button_inc_switch(ROTORY_ENCODER_PUSHBUTTON, 0, ROTORY_ENCODER_SWITCH_MAX, rotory_encoder.switch_value);
	else
		read_button_inc_switch(ROTORY_ENCODER_PUSHBUTTON, 0, ROTORY_ENCODER_SWITCH_MAX + 1, rotory_encoder.switch_value);
		*/
}

void read_button_neg_switch(byte button, bool &state)
{
	static unsigned long lastTime;
	unsigned long timeNow = millis();

	if (digitalRead(button) == 0) {
		if (timeNow - lastTime < BUTTON_DELAY)
			return;
		if (digitalRead(button) == 0)
		{
			state = !state;
		}
		lastTime = timeNow;
	}
}

void read_button_inc_switch(byte button, int limit_min, int limit_max, uint8_t& state)
{
	static unsigned long lastTime;
	unsigned long timeNow = millis();

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
	if (state > limit_max)
	{
		state = limit_min;
		//    Serial.print("Inc switch: ");
		//    Serial.println(state);
	}
	if (state < limit_min)
	{
		state = limit_max;
		//    Serial.print("Inc switch: ");
		//    Serial.println(state);
	}
}

void calibration()
{
	analog_correction.analog_left_X_correct = 128 - (analogRead(0) / 4);
	analog_correction.analog_left_Y_correct = 128 - (analogRead(1) / 4);
	analog_correction.analog_right_X_correct = 128 - (analogRead(2) / 4);
	analog_correction.analog_right_Y_correct = 128 - (analogRead(3) / 4);

	save_memory(0, 1, analog_correction.analog_left_X_correct + 128);
	save_memory(1, 1, analog_correction.analog_left_Y_correct + 128);
	save_memory(2, 1, analog_correction.analog_right_X_correct + 128);
	save_memory(3, 1, analog_correction.analog_right_Y_correct + 128);
}