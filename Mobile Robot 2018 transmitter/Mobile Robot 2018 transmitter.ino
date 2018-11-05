#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define MESSAGE_SEND_PERIOD 200

RF24 radio(8, 7);

const byte rxAddr[6] = { '1','N','o','d','e','0' };
byte outcoming_message[10];
byte message_counter = 0;

unsigned long long now, last_message_send;

void setup()
{
	Serial.begin(9600);

	radio.begin();
	radio.setRetries(15, 15);
	radio.setChannel(110);
	radio.openWritingPipe(rxAddr);

	//radio.stopListening();
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
		for (int i = 0; i <= 3; i++)
		{
			outcoming_message[i] = analogRead(i) / 4;
		}
		outcoming_message[sizeof(outcoming_message) - 1] = message_counter;
		message_counter++;
	}
}

void sendRadio(int period)
{
	if (now - last_message_send >= period)
	{
		last_message_send = now;
		radio.stopListening();
		radio.write(&outcoming_message, sizeof(outcoming_message));
		radio.startListening();
		printMessage();
	}
}

void printMessage()
{
	for (int i = 0; i < sizeof(outcoming_message); i++)
	{
		Serial.print(outcoming_message[i]);
		Serial.print(' ');
	}
	Serial.println();
}