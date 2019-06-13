void prepareOutMessage()
{


	int wait = 0;
	message.analog_left_Y = analogRead(1) / 4;
	//delay(wait);
	message.analog_left_X = analogRead(0) / 4;
	//delay(wait);
	message.analog_right_Y = analogRead(3) / 4;
	//delay(wait);
	message.analog_right_X = analogRead(2) / 4;
	//delay(wait);
	message.steering_wheel = analogRead(6) / 4;

	message.rotory_encoder = rotory_encoder.value;

	// Some magic to set bits in byte
	message.bit_array ^= (-digitalRead(SIDE_SWITCH) ^ message.bit_array) & (1UL << 0);
	message.bit_array ^= (-analog_left_switch_state ^ message.bit_array) & (1UL << 1);
	message.bit_array ^= (-analog_right_switch_state ^ message.bit_array) & (1UL << 2);
	message.bit_array ^= (-rotory_encoder.switch_state ^ message.bit_array) & (1UL << 3);


	outcoming_message[sizeof(outcoming_message) - 1] = message_counter;
	message.message_no = message_counter;
	message_counter++;

}

void sendRadio()
{

	last_message_send = now;
	radio.stopListening();
	radio.write(&message, sizeof(message));
	radio.startListening();

}

