#include <SPI.h>
//#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <EEPROM.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CE 7
#define CSN 8

#define MESSAGE_SEND_PERIOD 250
#define BUTTON_DELAY 400
#define ROTORY_ENCODER_CHANGE_MIN_TIME 50


#define SIDE_SWITCH 4
#define ANALOG_LEFT_PUSHBUTTON 2
#define ANALOG_RIGHT_PUSHBUTTON 3
#define ROTORY_ENCODER_PUSHBUTTON 9

#define ROTORY_ENCODER_CLK 6
#define ROTORY_ENCODER_DT 5

#define OLED_RESET 4

#define CONTROLS_NONE 0
#define CONTROLS_STANDARD 1
#define CONTROLS_ENCHANCED 2
#define CONTROLS_MEASURED 3
#define CONTROLS_AUTONOMUS 4

#define ROT_PB_EDIT_NONE 0
#define ROT_PB_EDIT_LX 1
#define ROT_PB_EDIT_LY 2
#define ROT_PB_EDIT_RX 3
#define ROT_PB_EDIT_RY 4
#define ROT_PB_EDIT_CTRL 5
#define ROT_PB_EDIT_CH 6
#define ROTORY_ENCODER_SWITCH_MAX 6

#define MENU_SIZE 5


//SERIAL OUTPUT

struct analogCorrection {
	int analog_left_X_correct = 0;
	int analog_left_Y_correct = 0;
	int analog_right_X_correct = 0;
	int analog_right_Y_correct = 0;
};

struct radioDataTrasnsmit {
	byte analog_left_X,
		analog_left_Y,
		analog_right_X,
		analog_right_Y,
		servo_0,
		led_g,
		led_b,
		potentiometer,
		control_mode,
		rotory_encoder,
		bit_array,
		message_no;
};

struct radioDataReceive {
	byte velocity_measured_left,
		velocity_measured_right,
		distance,
		control_mode,
		time_delay,
		reserved5,
		reserved6,
		reserved7,
		reserved8,
		message_no;
};

struct rotoryEncoder {
	bool clk_actual,
		 clk_prev,
		 dt_actual,
		 dt_prev,
		 switch_state;
	byte value,
		switch_value = 0,
		switch_value_old;
	int value_int;
};

struct radioStruct {
	byte last_message_no, 
		current_message_no, 
		messages_lost, 
		radio_not_availalble_counter;

	bool radio_not_availalble, empty_receive_data;
};

Adafruit_SSD1306 display(OLED_RESET);

/*
SimpleTimer 
*/
//radio variables
radioDataTrasnsmit message_transmit;
radioDataReceive message_receive;
radioStruct radioData;
uint8_t radio_channel = 0;
uint8_t control_mode = 0;

RF24 radio(CE, CSN);
const byte txAddr[6] = { '1','N','o','d','e','1' };
const byte rxAddr[6] = { '1','N','o','d','e','2' };

byte outcoming_message[6];
byte message_counter = 0;

unsigned long now,
	last_message_send,
	last_encloder_change,
	SerialRawTimer,
	SerialSwitchesTimer,
	SerialIncomingRadioTimer,
	PrepareMessageTimer,
	SendRadioTimer,
	RotoryEncoderTimer,
	DisplayUpdateTimer;

bool analog_left_switch_state, 
	analog_right_switch_state;

rotoryEncoder rotory_encoder;
analogCorrection analog_correction;

/*String menu_item_list[MENU_SIZE][MENU_SIZE] = {
	{"MENU1","MENU2","MENU3","MENU4","SPARE"},
	{"SUB11","SUB12","SUB13","SUB14","BACK"},
	{"SUB21","SUB22","SUB23","SUB24","BACK"},
	{"SUB31","SUB32","SUB33","SUB34","BACK"},
	{"SUB41","SUB42","SUB43","SUB44","BACK"},
};*/
byte menu_no = 0;


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

	//pinMode(A4, INPUT_PULLUP);
	//pinMode(A5, INPUT_PULLUP);

	analog_correction.analog_left_X_correct = int(get_memory(0, 1)) - 128;
	analog_correction.analog_left_Y_correct = int(get_memory(1, 1)) - 128;
	analog_correction.analog_right_X_correct = int(get_memory(2, 1)) - 128;
	analog_correction.analog_right_Y_correct = int(get_memory(3, 1)) - 128;
	radio_channel = get_memory(4, 1);
	control_mode = get_memory(5, 1);

	
	//---------------------- Radio config BEGIN -----------------
	
	radio.begin();
	radio.setDataRate(RF24_1MBPS);
	radio.setRetries(2, 5);
	radio.setChannel(radio_channel);// 100
	// -----------   JAK SIÊ ROZJEBIE TO ZMIEN KANA£ ---------
	radio.openWritingPipe(txAddr);
	radio.openReadingPipe(0, rxAddr);
	radio.stopListening();
	
	//---------------------- Radio config END -----------------

	//---------------------- OLED Display -----------------
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.clearDisplay();


	//---------------------- OLED Display END -----------------
	calibration();
}

void loop()
{
	now = millis();
	
	if (rotory_encoder.switch_value != rotory_encoder.switch_value_old && rotory_encoder.switch_value != 0)
	{
		switch (rotory_encoder.switch_value)
		{
		case ROT_PB_EDIT_LX:
			rotory_encoder.value_int = analog_correction.analog_left_X_correct;
			break;

		case ROT_PB_EDIT_LY:
			rotory_encoder.value_int = analog_correction.analog_left_Y_correct;
			break;

		case ROT_PB_EDIT_RX:
			rotory_encoder.value_int = analog_correction.analog_right_X_correct;
			break;

		case ROT_PB_EDIT_RY:
			rotory_encoder.value_int = analog_correction.analog_right_Y_correct;
			break;

		case ROT_PB_EDIT_CTRL:
			rotory_encoder.value_int = control_mode;
			break;			

		case ROT_PB_EDIT_CH:
			rotory_encoder.value_int = radio_channel;
			break;
		}
	}

	switch (rotory_encoder.switch_value)
	{
	case ROT_PB_EDIT_LX:
		analog_correction.analog_left_X_correct = rotory_encoder.value_int;
		save_memory(0, 1, analog_correction.analog_left_X_correct + 128);
		break;

	case ROT_PB_EDIT_LY:
		analog_correction.analog_left_Y_correct = rotory_encoder.value_int;
		save_memory(1, 1, analog_correction.analog_left_Y_correct + 128);
		break;

	case ROT_PB_EDIT_RX:
		analog_correction.analog_right_X_correct = rotory_encoder.value_int;
		save_memory(2, 1, analog_correction.analog_right_X_correct + 128);
		break;

	case ROT_PB_EDIT_RY:
		analog_correction.analog_right_Y_correct = rotory_encoder.value_int;
		save_memory(3, 1, analog_correction.analog_right_Y_correct + 128);
		break;

	case ROT_PB_EDIT_CTRL:
		if (rotory_encoder.value_int > 4)
			rotory_encoder.value_int = 0;
		if (rotory_encoder.value_int < 0)
			rotory_encoder.value_int = 4;
		control_mode = rotory_encoder.value_int;
		save_memory(5, 1, control_mode);
		break;

	case ROT_PB_EDIT_CH:
		if (rotory_encoder.value_int > 120)
			rotory_encoder.value_int = 0;
		if (rotory_encoder.value_int < 0)
			rotory_encoder.value_int = 120;
		radio_channel = rotory_encoder.value_int;
		save_memory(4, 1, radio_channel);
		break;

	}
	rotory_encoder.switch_value_old = rotory_encoder.switch_value;

	
	if (now - SerialIncomingRadioTimer > 1000)
	{
		SerialIncomingRadioTimer = now;
	}
	
	if (now - PrepareMessageTimer > 20)
	{
		prepareOutMessage();
		PrepareMessageTimer = now;
	}
	
	if (now - DisplayUpdateTimer > 700)
	{
		display_refresh();
		DisplayUpdateTimer = now;
	}

	readRadio(0);	
	tactileSwitchesHandler();	
	rotoryEncoderHandler();

	if (rotory_encoder.switch_value == ROTORY_ENCODER_SWITCH_MAX + 1)
		calibration();
	
}