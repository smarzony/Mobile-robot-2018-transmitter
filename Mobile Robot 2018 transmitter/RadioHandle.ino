void prepareOutMessage()
{


	int wait = 0;
	message_transmit.analog_left_Y = analogRead(1) / 4;
	//delay(wait);
	message_transmit.analog_left_X = analogRead(0) / 4;
	//delay(wait);
	message_transmit.analog_right_Y = analogRead(3) / 4;
	//delay(wait);
	message_transmit.analog_right_X = analogRead(2) / 4;
	//delay(wait);
	message_transmit.steering_wheel = analogRead(6) / 4;

	message_transmit.rotory_encoder = rotory_encoder.value;

	// Some magic to set bits in byte
	message_transmit.bit_array ^= (-digitalRead(SIDE_SWITCH) ^ message_transmit.bit_array) & (1UL << 0);
	message_transmit.bit_array ^= (-analog_left_switch_state ^ message_transmit.bit_array) & (1UL << 1);
	message_transmit.bit_array ^= (-analog_right_switch_state ^ message_transmit.bit_array) & (1UL << 2);
	message_transmit.bit_array ^= (-rotory_encoder.switch_state ^ message_transmit.bit_array) & (1UL << 3);


	outcoming_message[sizeof(outcoming_message) - 1] = message_counter;
	message_transmit.message_no = message_counter;
	message_counter++;

}

void sendRadio()
{
	last_message_send = now;
	radio.stopListening();
	radio.write(&message_transmit, sizeof(message_transmit));
	radio.startListening();
	
}


void readRadio(bool printing)
{
		if (radio.available())
		{
			radioData.last_message_no = radioData.current_message_no;
			radio.read(&message_receive, sizeof(message_receive));
			radioData.current_message_no = message_receive.message_no;
			radioData.messages_lost = radioData.current_message_no - radioData.last_message_no - 1;
			radioData.radio_not_availalble = 0;
			radioData.radio_not_availalble_counter = 0;
		}
		else
		{
			radioData.radio_not_availalble = 1;
			radioData.radio_not_availalble_counter++;
		}

		if (message_receive.velocity_measured_left == 0 && message_receive.velocity_measured_right &&
			message_receive.message_no == 0)
			radioData.empty_receive_data = 1;
		else
			radioData.empty_receive_data = 0;
			
}
