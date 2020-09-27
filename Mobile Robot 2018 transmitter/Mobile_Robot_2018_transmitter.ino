
#define NODEMCU
#define WIFI_STATION
#define CALIBRATE
// #define WIFI_ACCESS_POINT
// #define PROMINI

#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>

#include "timer.h"
#include "pinstate.h"
#include "buttons.h"

#ifdef NODEMCU
  #include <jm_PCF8574.h>
  #include <Adafruit_ADS1015.h>
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif


#ifdef NODEMCU
//PHYSICAL
  #define CE D3
  #define CSN D4
  #define POTENTIOMETER A0

  //PCF8574
  #define ANALOG_LEFT_PUSHBUTTON 0
  #define ANALOG_RIGHT_PUSHBUTTON 1
  #define SIDE_SWITCH 2
  #define ROTORY_ENCODER_PUSHBUTTON 3
  #define ROTORY_ENCODER_CLK 4
  #define ROTORY_ENCODER_DT 5

  #define BUTTON_PLUS 3
  #define BUTTON_SELECT 4
  #define BUTTON_MINUS 5

  //ADS1015
  #define ANALOG_LEFT_X 2
  #define ANALOG_LEFT_Y 3
  #define ANALOG_RIGHT_X 0
  #define ANALOG_RIGHT_Y 1

#endif



#ifdef PROMINI
  #define SIDE_SWITCH 7
  #define ANALOG_LEFT_PUSHBUTTON 2
  #define ANALOG_RIGHT_PUSHBUTTON 3
  #define ROTORY_ENCODER_PUSHBUTTON 4

  #define ANALOG_LEFT_X 0
  #define ANALOG_LEFT_Y 1

  #define ANALOG_RIGHT_X 2
  #define ANALOG_RIGHT_Y 3

  #define POTENTIOMETER 6

  #define ROTORY_ENCODER_CLK 5
  #define ROTORY_ENCODER_DT 6
#endif

#define OLED_RESET -1

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
#define BUTTON_DELAY 300
#define ROTORY_ENCODER_CHANGE_MIN_TIME 50



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

#ifdef NODEMCU
jm_PCF8574 remoteIO(0x20);
Adafruit_ADS1015 remoteAI; 
PinState buttons[3];
#endif

//radio variables
radioDataTrasnsmit message_transmit;
radioDataReceive message_receive;
radioStruct radioData;
uint8_t radio_channel = 0;
uint8_t control_mode = 0;

RF24 radio(CE, CSN);
const byte txAddr[6] = { '1', 'N', 'o', 'd', 'e', '1' };
const byte rxAddr[6] = { '1', 'N', 'o', 'd', 'e', '2' };

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

byte menu_no = 0;

void setup()
{

  //---------------------- DIGITAL REMOTE PINS
  #ifdef NODEMCU
    prepareOTA();
    remoteAI.setGain(GAIN_TWOTHIRDS);
    remoteAI.begin();
    remoteIO.begin(0x20);
    remoteIO.pinMode(SIDE_SWITCH, INPUT_PULLUP);
    remoteIO.pinMode(ANALOG_LEFT_PUSHBUTTON, INPUT_PULLUP);
    remoteIO.pinMode(ANALOG_RIGHT_PUSHBUTTON, INPUT_PULLUP);

    // remoteIO.pinMode(ROTORY_ENCODER_PUSHBUTTON, INPUT_PULLUP);
    // remoteIO.pinMode(ROTORY_ENCODER_CLK, INPUT_PULLUP);
    // remoteIO.pinMode(ROTORY_ENCODER_DT, INPUT_PULLUP);

    remoteIO.pinMode(BUTTON_MINUS, INPUT_PULLUP);
    remoteIO.pinMode(BUTTON_SELECT, INPUT_PULLUP);
    remoteIO.pinMode(BUTTON_PLUS, INPUT_PULLUP);
    buttons[0].no = BUTTON_MINUS;
    buttons[1].no = BUTTON_SELECT;
    buttons[2].no = BUTTON_PLUS;
  #endif

  #ifdef PROMINI
    pinMode(A4, INPUT_PULLUP);
    pinMode(A5, INPUT_PULLUP);
    pinMode(SIDE_SWITCH, INPUT_PULLUP);
    pinMode(ANALOG_LEFT_PUSHBUTTON, INPUT_PULLUP);
    pinMode(ANALOG_RIGHT_PUSHBUTTON, INPUT_PULLUP);

    pinMode(ROTORY_ENCODER_PUSHBUTTON, INPUT_PULLUP);
    pinMode(ROTORY_ENCODER_CLK, INPUT_PULLUP);
    pinMode(ROTORY_ENCODER_DT, INPUT_PULLUP);
  #endif


  #ifdef CALIBRATE
    // save_memory(0, 1, 128);
    // save_memory(1, 1, 128);
    // save_memory(2, 1, 128);
    // save_memory(3, 1, 128);
    analog_correction.analog_left_X_correct = int(get_memory(0, 1)) - 128;
    analog_correction.analog_left_Y_correct = int(get_memory(1, 1)) - 128;
    analog_correction.analog_right_X_correct = int(get_memory(2, 1)) - 128;
    analog_correction.analog_right_Y_correct = int(get_memory(3, 1)) - 128;
  #endif
  radio_channel = get_memory(4, 1);
  control_mode = get_memory(5, 1);

  //---------------------- Radio config BEGIN -----------------

  radio.begin();
  radio.setDataRate(RF24_1MBPS);
  radio.setRetries(2, 5);
  radio.setChannel(radio_channel);// 100
  // -----------   JAK SI� ROZJEBIE TO ZMIEN KANA� ---------
  radio.openWritingPipe(txAddr);
  radio.openReadingPipe(0, rxAddr);
  radio.stopListening();

  //---------------------- Radio config END -----------------

  //---------------------- OLED Display -----------------
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();


  //---------------------- OLED Display END -----------------
  #ifdef CALIBRATE
    calibration();
  #endif

}

void loop()
{
  #ifdef NODEMCU
    ArduinoOTA.handle();
  #endif
  now = millis();

  buttons[0].last = buttons[0].actual;
  buttons[1].last = buttons[1].actual;
  buttons[2].last = buttons[2].actual;
  buttons[0].actual = remoteIO.digitalRead(buttons[0].no);
  buttons[1].actual = remoteIO.digitalRead(buttons[1].no);
  buttons[2].actual = remoteIO.digitalRead(buttons[2].no);

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

  if (now - PrepareMessageTimer > 20)
  {
    prepareOutMessage();
    PrepareMessageTimer = now;
  }

  if (now - DisplayUpdateTimer > 500)
  {
    DisplayUpdateTimer = now;
    display_refresh();    
  }


  if (now - SerialRawTimer > 500)
  {
    SerialRawTimer = now;
    serialPrintRaw();
  }

  readRadio(0);
  // tactileSwitchesHandler();
  read_button_neg_switch(ANALOG_LEFT_PUSHBUTTON, analog_left_switch_state);
  read_button_neg_switch(ANALOG_RIGHT_PUSHBUTTON, analog_right_switch_state);
  read_button_inc_switch(BUTTON_SELECT, 0, ROTORY_ENCODER_SWITCH_MAX, rotory_encoder.switch_value, remoteIO.digitalRead);
  
  button_hold(buttons[0], rotory_encoder.value_int, substract);
  button_hold(buttons[2], rotory_encoder.value_int, add);
  // rotoryEncoderHandler();

  if (rotory_encoder.switch_value == 6)
    calibration();

}

void substract(int sub_value, int &input)
{
  input = input - sub_value;
}

void add(int add_value, int &input)
{
  input = input + add_value;
}
