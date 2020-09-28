#include "serial_print.h"

void serialPrintTx(radioDataTrasnsmit &message_transmit)
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
    	
	Serial.println(output);		
}

void serialPrintPCF(jm_PCF8574 &remoteIO)
{
	String output;
	for (uint8_t x = 0; x <= 7; x++)
	{
		output += String(remoteIO.digitalRead(x));
		output += " ";
	}
	Serial.println("PCF: "+ output);
}