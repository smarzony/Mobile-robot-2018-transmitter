void serialPrintRaw()
{
	String output;
	output += message_transmit.analog_left_X;
	output += ' ';
	output += message_transmit.analog_left_Y;
	output += ' ';
	output += message_transmit.analog_right_X;
	output += ' ';
	output += message_transmit.analog_right_Y;
	output += ' ';
	output += message_transmit.potentiometer;
	output += ' ';
	output += message_transmit.rotory_encoder;
	output += ' ';
    	
	Serial.println(output);
	
	
}

void serialPrintSwitches()
{
//	String output;
//	Serial.println(output);
}

void serialPrintIncomingMessage()
{
//	String output;
//	output += message_receive.velocity_measured_left;
//	output += ' ';
//	output += message_receive.velocity_measured_right;
//	output += ' ';
//	output += message_receive.distance;
//	output += ' ';
//	output += message_receive.control_mode;
//	output += ' ';
//	output += message_receive.message_no;
//
//
//	Serial.println(output);
}
