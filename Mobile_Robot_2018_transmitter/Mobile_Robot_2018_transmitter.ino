/*
Mobile robot transmitter code
Author: Piotr Smarzyński
*/

#define NODEMCU
#define WIFI_STATION
// #define CALIBRATE
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
#include "analog_correction.h"
#include "radio_data.h"
#include "calibration.h"
#include "memory.h"
#include "serial_print.h"

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
  #define CE D0
  #define CSN D8
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
jm_PCF8574 remoteIO;
jm_PCF8574 remoteIO1;
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

void prepareOutMessage()
{
  int value_send;

  #ifdef NODEMCU
    value_send = remoteAI.readADC_SingleEnded(ANALOG_LEFT_X )/ 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_left_X_correct;
    #endif
    message_transmit.analog_left_X = byte_limit(value_send);

    value_send = remoteAI.readADC_SingleEnded(ANALOG_LEFT_Y )/ 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_left_Y_correct;
    #endif
    message_transmit.analog_left_Y = byte_limit(value_send);

    value_send = remoteAI.readADC_SingleEnded(ANALOG_RIGHT_X )/ 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_right_X_correct;
    #endif
    message_transmit.analog_right_X = byte_limit(value_send);

    value_send = remoteAI.readADC_SingleEnded(ANALOG_RIGHT_Y )/ 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_right_Y_correct;
    #endif
    message_transmit.analog_right_Y = byte_limit(value_send);
  #endif

  #ifdef PROMINI
    value_send = analogRead(ANALOG_LEFT_X) / 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_left_X_correct;
    #endif
    message_transmit.analog_left_X = byte_limit(value_send);

    value_send = analogRead(ANALOG_LEFT_Y) / 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_left_Y_correct;
    #endif
    message_transmit.analog_left_Y = byte_limit(value_send);

    value_send = analogRead(ANALOG_RIGHT_X) / 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_right_X_correct;
    #endif
    message_transmit.analog_right_X = byte_limit(value_send);

    value_send = analogRead(ANALOG_RIGHT_Y) / 4;
    #ifdef CALIBRATE
     value_send = value_send  + analog_correction.analog_right_Y_correct;
    #endif
    message_transmit.analog_right_Y = byte_limit(value_send);
  #endif
  

  message_transmit.control_mode = control_mode;

  message_transmit.potentiometer = analogRead(POTENTIOMETER) / 4;
  message_transmit.rotory_encoder = rotory_encoder.value;

  // Some magic to set bits in byte
  #ifdef PROMINI
    message_transmit.bit_array ^= (-digitalRead(SIDE_SWITCH) ^ message_transmit.bit_array) & (1UL << 0);
  #endif
  #ifdef NODEMCU
    message_transmit.bit_array ^= (-remoteIO.digitalRead(SIDE_SWITCH) ^ message_transmit.bit_array) & (1UL << 0);
  #endif
  message_transmit.bit_array ^= (-analog_left_switch_state ^ message_transmit.bit_array) & (1UL << 1);
  message_transmit.bit_array ^= (-analog_right_switch_state ^ message_transmit.bit_array) & (1UL << 2);
  message_transmit.bit_array ^= (-rotory_encoder.switch_state ^ message_transmit.bit_array) & (1UL << 3);


  outcoming_message[sizeof(outcoming_message) - 1] = message_counter;
  message_transmit.message_no = message_counter;
  message_counter++;

  sendRadio();
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

void setup()
{  
  //---------------------- DIGITAL REMOTE PINS
  #ifdef NODEMCU
    EEPROM.begin(32);
    prepareOTA();
    remoteAI.setGain(GAIN_TWOTHIRDS);
    remoteAI.begin();
    remoteIO.begin(0x20);
    remoteIO.begin(0x21);

    for(int pin=0; pin<=7; pin++)
    {
      remoteIO.pinMode(pin, INPUT_PULLUP);
      remoteIO.digitalWrite(pin, HIGH);
      remoteIO1.pinMode(pin, INPUT_PULLUP);
      remoteIO1.digitalWrite(pin, HIGH);
    }
    // remoteIO1 PIN3 is not working!



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
    analog_correction.analog_left_X_correct = int(get_memory(0, 1)) - 128;
    analog_correction.analog_left_Y_correct = int(get_memory(1, 1)) - 128;
    analog_correction.analog_right_X_correct = int(get_memory(2, 1)) - 128;
    analog_correction.analog_right_Y_correct = int(get_memory(3, 1)) - 128;
  #endif
  radio_channel = get_memory(4, 1);
  // radio_channel = 0;
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
    calibration(analog_correction);
  #endif
  Serial.begin(115200);
  delay(500);
  Serial.println('radio_channel: '+String(radio_channel));
  Serial.println('control_mode: '+String(control_mode));

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

  readRadio(0);
  read_button_neg_switch(ANALOG_LEFT_PUSHBUTTON, analog_left_switch_state, remoteIO);
  read_button_neg_switch(ANALOG_RIGHT_PUSHBUTTON, analog_right_switch_state, remoteIO);
  read_button_inc_switch(BUTTON_SELECT, 0, ROTORY_ENCODER_SWITCH_MAX, rotory_encoder.switch_value, remoteIO);
  
  button_hold(buttons[0], rotory_encoder.value_int, substract);
  button_hold(buttons[2], rotory_encoder.value_int, add);

  if (rotory_encoder.switch_value == 6)
    calibration(analog_correction);

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
      // radio_channel = rotory_encoder.value_int;
      save_memory(4, 1, radio_channel);
      break;

  }
  rotory_encoder.switch_value_old = rotory_encoder.switch_value;

  if (now - PrepareMessageTimer > 20)
  {
    PrepareMessageTimer = now;
    prepareOutMessage();    
  }

  if (now - DisplayUpdateTimer > 200)
  {
    DisplayUpdateTimer = now;
    display_refresh();    
  }


  if (now - SerialRawTimer > 500)
  {
    SerialRawTimer = now;
    // serialPrintTx(message_transmit);
    // serialPrintPCF(remoteIO);
    // Serial.println("radio_connected: "+String(radio.isChipConnected()));
  }
}

void substract(int sub_value, int &input)
{
  input = input - sub_value;
}

void add(int add_value, int &input)
{
  input = input + add_value;
}
