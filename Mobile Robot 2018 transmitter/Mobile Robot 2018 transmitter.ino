#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 7);

const byte rxAddr[6] = "00001";

void setup()
{
	radio.begin();
	radio.setRetries(15, 15);
	radio.openWritingPipe(rxAddr);

	radio.stopListening();
}

void loop()
{
	const char text[] = "Witam";
	radio.write(&text, sizeof(text));

	delay(1000);
}