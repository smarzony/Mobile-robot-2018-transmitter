void serialPrintRaw()
{
	String output;
	output += message.analog_left_X;
	output += ' ';
	output += message.analog_left_Y;
	output += ' ';
	output += message.analog_right_X;
	output += ' ';
	output += message.analog_right_Y;
	output += ' ';
	output += message.steering_wheel;
	output += ' ';
	output += message.rotory_encoder;
	output += ' ';
	output += "[S, L, R, RE]: ";
	for (int counter = 0; counter < 8; counter++)
	{
		output += bitRead(message.bit_array, counter);
	}
	
	Serial.println(output);
	
	
}

void serialPrintSwitches()
{
	String output;

	Serial.println(output);
}