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

/*
void readRadio(int period, bool printing)
{

	if ((now - last_message_read >= period))
	{
		if (radio.available())
		{
			last_message_no = current_message_no;

			radio.read(&message_receive, sizeof(message_receive));
			current_message_no = message_receive.message_no;
			messages_lost = current_message_no - last_message_no - 1;
			if (printing)
				printMessage(RADIO_PRINT_INCOMING_MESSAGE);
			radio_not_availalble = 0;
			radio_not_availalble_counter = 0;

			side_switch = bitRead(message_receive.bit_array, 0);
			analog_left_switch = bitRead(message_receive.bit_array, 1);
			analog_right_switch = bitRead(message_receive.bit_array, 2);
			rotory_encoder_switch = bitRead(message_receive.bit_array, 3);
		}
		else
		{
			radio_not_availalble = 1;
			radio_not_availalble_counter++;
		}

		if (message_receive.analog_left_Y == 0 && message_receive.analog_left_X &&
			message_receive.analog_right_X == 0 & message_receive.analog_right_Y == 0)
			empty_receive_data = 1;
		else
			empty_receive_data = 0;

	}
}
*/