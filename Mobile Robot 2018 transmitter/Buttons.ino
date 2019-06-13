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

void tactileSwitchesHandler()
{
	read_button_neg_switch(ANALOG_LEFT_PUSHBUTTON, analog_left_switch_state);
	read_button_neg_switch(ANALOG_RIGHT_PUSHBUTTON, analog_right_switch_state);
	read_button_neg_switch(ROTORY_ENCODER_PUSHBUTTON, rotory_encoder.switch_state);
}