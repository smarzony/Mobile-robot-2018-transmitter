#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define MESSAGE_SEND_PERIOD 200

struct radioData {
	byte analog_left_X;
	byte analog_left_Y;
	byte analog_right_X;
	byte analog_right_Y;
	byte led_r;
	byte led_g;
	byte led_b;
	byte reserved0;
	byte reserved1;
	byte reserved2;
	byte reserved3;
	byte message_no;
};

//radio variables
radioData message;
RF24 radio(7,8);
const byte rxAddr[6] = { '1','N','o','d','e','1' };

byte outcoming_message[6];
byte message_counter = 0;
unsigned long now, last_message_send;

void setup()
{
	Serial.begin(9600);
	Serial.println("Starting...");
	//---------------------- Radio config BEGIN -----------------
	radio.begin();
	radio.setDataRate(RF24_1MBPS);
	radio.setRetries(2, 5);
	radio.setChannel(0);// 100
	// -----------   JAK SIÊ ROZJEBIE TO ZMIEN KANA£ ---------
	radio.openWritingPipe(rxAddr);
	radio.stopListening();

	//---------------------- Radio config END -----------------
}

void loop()
{
	now = millis();
	prepareOutMessage(MESSAGE_SEND_PERIOD);
	sendRadio(MESSAGE_SEND_PERIOD);
	
}

void prepareOutMessage(int period)
{
	if (now - last_message_send >= period)
	{
		message.analog_left_Y =		analogRead(0)/4;
		message.analog_left_X =		analogRead(1)/4;
		message.analog_right_Y =	analogRead(2)/4;
		message.analog_right_X =	analogRead(3)/4;
		/*
		for (int i = 0; i <= 3; i++)
		{
			outcoming_message[i] = analogRead(i) / 4;
		}*/
		outcoming_message[sizeof(outcoming_message) - 1] = message_counter;
		message.message_no = message_counter;
		message_counter++;
	}
}

void sendRadio(int period)
{
	if (now - last_message_send >= period)
	{
		last_message_send = now;
		radio.stopListening();
		//radio.write(&outcoming_message, sizeof(outcoming_message));
		radio.write(&message, sizeof(message));
		radio.startListening();
		printMessage();
	}
}

void printMessage()
{
	/*
	for (int i = 0; i < sizeof(outcoming_message); i++)
	{
		Serial.print(outcoming_message[i]);
		Serial.print(' ');
	}*/

	int ms, seconds, minutes, hours;

	ms = millis() % 1000;
	seconds = ( millis() / 1000 ) % 60;
	minutes = ( millis() / (1000*60) ) % 60;
	//hours = (millis() / (1000 * 60 * 60)) % 24;
	
	Serial.print(message.analog_left_X);
	Serial.print(' ');
	Serial.print(message.analog_left_Y);
	Serial.print(' ');
	Serial.print(message.analog_right_X);
	Serial.print(' ');
	Serial.print(message.analog_right_Y);
	Serial.print(' ');
	Serial.print(message.message_no);
	Serial.print(" \tt: ");
	//Serial.print(hours);
	//Serial.print(':');
	Serial.print(minutes);
	Serial.print(':');
	Serial.print(seconds);
	Serial.print('.');


	if (ms < 10)
		Serial.print("00");
	if (ms > 10 && ms < 100)
		Serial.print('0');

	Serial.print(ms);

	Serial.print(" Conn: ");
	Serial.println(radio.isChipConnected());
	
}