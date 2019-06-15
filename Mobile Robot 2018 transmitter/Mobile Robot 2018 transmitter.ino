#include <SimpleTimer.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE 7
#define CSN 8

#define MESSAGE_SEND_PERIOD 250
#define BUTTON_DELAY 400
#define ROTORY_ENCODER_CHANGE_MIN_TIME 150

#define SIDE_SWITCH 4
#define ANALOG_LEFT_PUSHBUTTON 2
#define ANALOG_RIGHT_PUSHBUTTON 3
#define ROTORY_ENCODER_PUSHBUTTON 9

#define ROTORY_ENCODER_CLK 6
#define ROTORY_ENCODER_DT 5

//SERIAL OUTPUT


struct radioDataTrasnsmit {
	byte analog_left_X;
	byte analog_left_Y;
	byte analog_right_X;
	byte analog_right_Y;
	byte led_r;
	byte led_g;
	byte led_b;
	byte steering_wheel;
	byte reserved1;
	byte rotory_encoder;
	byte bit_array;
	byte message_no;
};

struct radioDataReceive {
	byte reserved0,
		reserved1,
		reserved2,
		reserved3,
		reserved4,
		reserved5,
		reserved6,
		reserved7,
		reserved8,
		message_no;
};

struct rotoryEncoder {
	bool clk_actual;
	bool clk_prev;
	bool dt_actual;
	bool dt_prev;
	bool switch_state;
	byte value;
};

SimpleTimer SerialRawTimer, SerialSwitchesTimer, PrepareMessageTimer, SendRadioTimer, RotoryEncoderTimer;

//radio variables
radioDataTrasnsmit message_transmit;
radioDataReceive message_receive;
RF24 radio(CE, CSN);
const byte txAddr[6] = { '1','N','o','d','e','1' };
const byte rxAddr[6] = { '1','N','o','d','e','2' };

byte outcoming_message[6];
byte message_counter = 0;
unsigned long now, last_message_send, last_encloder_change;

bool analog_left_switch_state, analog_right_switch_state;

rotoryEncoder rotory_encoder;

void setup()
{
	Serial.begin(9600);
	Serial.println("Starting...");

	pinMode(SIDE_SWITCH, INPUT_PULLUP);
	pinMode(ANALOG_LEFT_PUSHBUTTON, INPUT_PULLUP);
	pinMode(ANALOG_RIGHT_PUSHBUTTON, INPUT_PULLUP);

	pinMode(ROTORY_ENCODER_PUSHBUTTON, INPUT_PULLUP);
	pinMode(ROTORY_ENCODER_CLK, INPUT_PULLUP);
	pinMode(ROTORY_ENCODER_DT, INPUT_PULLUP);
	//---------------------- Radio config BEGIN -----------------
	radio.begin();
	radio.setDataRate(RF24_1MBPS);
	radio.setRetries(2, 5);
	radio.setChannel(0);// 100
	// -----------   JAK SIÊ ROZJEBIE TO ZMIEN KANA£ ---------
	radio.openWritingPipe(txAddr);
	radio.openReadingPipe(0, rxAddr);
	radio.stopListening();

	//---------------------- Radio config END -----------------
	SerialRawTimer.setInterval(500, serialPrintRaw);	
	PrepareMessageTimer.setInterval(200, prepareOutMessage);
	SendRadioTimer.setInterval(250, sendRadio);
}

void loop()
{
	now = millis();
	PrepareMessageTimer.run();
	SendRadioTimer.run();
	SerialRawTimer.run();
	
	//Rotory encoder and tactile switches works better this way
	tactileSwitchesHandler();
	rotoryEncoderHandler();
}


