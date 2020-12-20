#include "Arduino.h"

// void display_refresh()
// {
// 	display.setTextSize(1);
// 	display.setTextColor(WHITE);

// 	// LEFT COLUMN
// 	byte column = 0;
// 	byte line = 0;
// 	byte space = 0;

// 	display.setCursor(column, line);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_LX)
// 		display.setTextColor(BLACK, WHITE);
// 	display.print("LX : ");
// 	display.println(message_transmit.analog_left_X);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_LX)
// 		display.setTextColor(WHITE);

// 	line = line + 10;
// 	display.setCursor(column, line);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_LY)
// 		display.setTextColor(BLACK, WHITE);
// 	display.print("LY : ");
// 	display.println(message_transmit.analog_left_Y);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_LY)
// 		display.setTextColor(WHITE);

// 	line = line + 10;
// 	display.setCursor(column, line);
// 	display.print("LSW: ");
// 	display.println(analog_left_switch_state);

// 	line = line + 10;
// 	display.setCursor(column, line);
// 	display.print("SID: ");
// 	#ifdef PROMINI
// 		display.println(digitalRead(SIDE_SWITCH));
// 	#endif
// 	#ifdef NODEMCU
// 		display.println(remoteIO.digitalRead(SIDE_SWITCH));
// 	#endif


// 	line = line + 10;
// 	display.setCursor(column, line);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_CTRL)
// 		display.setTextColor(BLACK, WHITE);
// 	display.print("MOD: ");

// 	String output;
// 	switch (control_mode)
// 	{
// 	case CONTROLS_STANDARD:
// 		output = "STD";
// 		break;

// 	case CONTROLS_ENCHANCED:
// 		output = "ENCH";
// 		break;

// 	case CONTROLS_MEASURED:
// 		output = "MSRD";
// 		break;

// 	case CONTROLS_AUTONOMUS:
// 		output = "AUTO";
// 		break;

// 	default:
// 		output = "NONE";
// 	}
// 	display.println(output);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_CTRL)
// 		display.setTextColor(WHITE);

// 	// RIGHT COLUMN
// 	column = 64;
// 	line = 0;
// 	/*
// 	display.setTextColor(BLACK, WHITE);
// 	display.setTextColor(WHITE);
// 	*/
// 	display.setCursor(column, line);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_RX)
// 		display.setTextColor(BLACK, WHITE);
// 	display.print("RX : ");	
// 	display.println(message_transmit.analog_right_X);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_RX)
// 		display.setTextColor(WHITE);

// 	line = line + 10;
// 	display.setCursor(column, line);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_RY)
// 		display.setTextColor(BLACK, WHITE);
// 	display.print("RY : ");	
// 	display.println(message_transmit.analog_right_Y);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_RY)
// 		display.setTextColor(WHITE);

// 	line = line + 10;
// 	display.setCursor(column, line);
// 	display.print("RSW: ");
// 	display.println(analog_right_switch_state);

// 	line = line + 10;
// 	display.setCursor(column, line);
// 	display.print("POT: ");
// 	display.println(message_transmit.potentiometer);

// 	line = line + 10;
// 	display.setCursor(column, line);
// 	display.print("DIS: ");
// 	display.println(message_receive.distance);
//   	// display.print("RSV: ");
//   	// display.println(rotory_encoder.switch_value);



// 	// UPPER BOTTOM BAR
// //	
// //	if (rotory_encoder.switch_value > 0 && rotory_encoder.switch_value <= ROTORY_ENCODER_SWITCH_MAX)
// //	{
// //		line = 48;
// //		column = 0;
// //		space = 20;
// //
// //		display.setCursor(column, line);
// //		display.print("MEM: ");
// //
// //		column = column + 24;
// //		display.setCursor(column, line);
// //		display.setTextColor(BLACK, WHITE);
// //		display.print(get_memory(0, 1));
// //		display.setTextColor(WHITE);
// //
// //		column = column + space;
// //		display.setCursor(column, line);
// //		display.setTextColor(BLACK, WHITE);
// //		display.print(get_memory(1, 1));
// //		display.setTextColor(WHITE);
// //
// //		column = column + space;
// //		display.setCursor(column, line);
// //		display.setTextColor(BLACK, WHITE);
// //		display.print(get_memory(2, 1));
// //		display.setTextColor(WHITE);
// //
// //		column = column + space;
// //		display.setCursor(column, line);
// //		display.setTextColor(BLACK, WHITE);
// //		display.print(get_memory(3, 1));
// //		display.setTextColor(WHITE);
// //
// //		column = column + space;
// //		display.setCursor(column, line);
// //		display.setTextColor(BLACK, WHITE);
// //		display.print(get_memory(4, 1));
// //		display.setTextColor(WHITE);
// //	}
	
// 	if (rotory_encoder.switch_value == ROTORY_ENCODER_SWITCH_MAX + 1)
// 	{
// 		line = 47;
// 		column = 20;
// 		display.setCursor(column, line);
// 		display.setTextColor(BLACK, WHITE);
// 		display.print("CALIBRATION");
// 		display.setTextColor(WHITE);
// 	}

// 	// BOTTOM BAR
// 	line = 57;
// 	column = 0;
// 	display.setCursor(column, line);
// 	display.print("Tx");

// 	column = column + 13;
// 	display.setCursor(column, line);
// 	display.print(message_counter);

// 	column = column + 22;
// 	display.setCursor(column, line);
// 	display.print("Rx");

// 	column = column + 13;
// 	display.setCursor(column, line);
// 	display.print(message_receive.message_no);

// 	column = column + 22;	
// 	display.setCursor(column, line);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_CH)
// 		display.setTextColor(BLACK, WHITE);
// 	display.print("CH");
	
// 	column = column + 13;	
// 	display.setCursor(column, line);
// 	display.print(radio_channel);
// 	if (rotory_encoder.switch_value == ROT_PB_EDIT_CH)
// 		display.setTextColor(WHITE);

	

// 	/*
// 	display.print("DEL");
// 	display.setCursor(92, line);
// 	display.println(message_receive.time_delay);
// 	*/

// 	display.display();
// 	display.clearDisplay();
// }
