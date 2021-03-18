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
  #define ANALOG_LEFT_PUSHBUTTON 7
  #define ANALOG_RIGHT_PUSHBUTTON 6
  #define SIDE_SWITCH 2
  #define ROTORY_ENCODER_PUSHBUTTON 3
  #define ROTORY_ENCODER_CLK 4
  #define ROTORY_ENCODER_DT 5

  #define BUTTON_PLUS 2
  #define BUTTON_SELECT 1
  #define BUTTON_MINUS 0

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

struct menu {
  int option;
  int selected;
  int value;
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
jm_PCF8574 remoteIO;
jm_PCF8574 remoteIO1;
Adafruit_ADS1015 remoteAI; 
PinState button_minus, button_select, button_plus;
#endif

//radio variables
radioDataTrasnsmit message_transmit;
radioDataReceive message_receive;
radioStruct radioData;
uint8_t radio_channel = 0;
uint8_t control_mode = 1;

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
uint8_t print_counter = 0;
menu gui_menu;

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
    // prepareOTA();
    remoteAI.setGain(GAIN_TWOTHIRDS);
    remoteAI.begin(); // 0x48

    remoteIO.begin(0x20);
    remoteIO1.begin(0x21);

    for(int pin=0; pin<=7; pin++)
    {
      remoteIO.pinMode(pin, INPUT_PULLUP);
      remoteIO.digitalWrite(pin, HIGH);
      delay(50);
      remoteIO1.pinMode(pin, INPUT_PULLUP);
      remoteIO1.digitalWrite(pin, HIGH);
      delay(50);
    }
    // remoteIO1 PIN3 is not working!

    // buttons[0].no = BUTTON_MINUS;
    // buttons[1].no = BUTTON_SELECT;
    // buttons[2].no = BUTTON_PLUS;

    button_minus.no = BUTTON_MINUS;
    button_select.no = BUTTON_SELECT;
    button_plus.no = BUTTON_PLUS;
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

  //---------------------- Radio config BEGIN -----------------

  Serial.begin(115200);
  radio_channel = get_memory(4, 1);
  if (radio_channel > 120)
    {
      radio_channel = 0;
    }
  control_mode = get_memory(5, 1);
  if (control_mode > CONTROLS_AUTONOMUS)
  {
    control_mode = CONTROLS_NONE;
  }

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


  
  delay(500);  
  Serial.println("\nradio_channel: "+String(radio_channel));
  Serial.println("control_mode: "+String(control_mode));

  gui_menu.option = 0;

}

void loop()
{
  #ifdef NODEMCU
    // ArduinoOTA.handle();
  #endif
  now = millis();

  // buttons[0].last = buttons[0].actual;
  // buttons[1].last = buttons[1].actual;
  // buttons[2].last = buttons[2].actual;
  // buttons[0].actual = remoteIO.digitalRead(buttons[0].no);
  // buttons[1].actual = remoteIO.digitalRead(buttons[1].no);
  // buttons[2].actual = remoteIO.digitalRead(buttons[2].no); 

  button_minus.last = button_minus.actual;
  button_select.last = button_select.actual;
  button_plus.last = button_plus.actual;
  button_minus.actual = remoteIO.digitalRead(button_minus.no);
  button_select.actual = remoteIO.digitalRead(button_select.no);
  button_plus.actual = remoteIO.digitalRead(button_plus.no); 


  readRadio(0);
  read_button_neg_switch(ANALOG_LEFT_PUSHBUTTON, analog_left_switch_state, remoteIO1);
  read_button_neg_switch(ANALOG_RIGHT_PUSHBUTTON, analog_right_switch_state, remoteIO1);

  menu();

  // read_button_inc_switch(BUTTON_SELECT, 0, ROTORY_ENCODER_SWITCH_MAX, rotory_encoder.switch_value, remoteIO);  


  // if (rotory_encoder.switch_value == 6)
  //   calibration(analog_correction);

  // if (rotory_encoder.switch_value != rotory_encoder.switch_value_old && rotory_encoder.switch_value != 0)
  // {
  //   switch (rotory_encoder.switch_value)
  //   {
  //     case ROT_PB_EDIT_LX:
  //       rotory_encoder.value = analog_correction.analog_left_X_correct;
  //       break;

  //     case ROT_PB_EDIT_LY:
  //       rotory_encoder.value = analog_correction.analog_left_Y_correct;
  //       break;

  //     case ROT_PB_EDIT_RX:
  //       rotory_encoder.value = analog_correction.analog_right_X_correct;
  //       break;

  //     case ROT_PB_EDIT_RY:
  //       rotory_encoder.value = analog_correction.analog_right_Y_correct;
  //       break;

  //     case ROT_PB_EDIT_CTRL:
  //       rotory_encoder.value = control_mode;
  //       break;

  //     case ROT_PB_EDIT_CH:
  //       rotory_encoder.value = radio_channel;
  //       break;
  //   }
  // }

  // switch (rotory_encoder.switch_value)
  // {
  //   case ROT_PB_EDIT_LX:
  //     analog_correction.analog_left_X_correct = rotory_encoder.value;
  //     save_memory(0, 1, analog_correction.analog_left_X_correct + 128);
  //     break;

  //   case ROT_PB_EDIT_LY:
  //     analog_correction.analog_left_Y_correct = rotory_encoder.value;
  //     save_memory(1, 1, analog_correction.analog_left_Y_correct + 128);
  //     break;

  //   case ROT_PB_EDIT_RX:
  //     analog_correction.analog_right_X_correct = rotory_encoder.value;
  //     save_memory(2, 1, analog_correction.analog_right_X_correct + 128);
  //     break;

  //   case ROT_PB_EDIT_RY:
  //     analog_correction.analog_right_Y_correct = rotory_encoder.value;
  //     save_memory(3, 1, analog_correction.analog_right_Y_correct + 128);
  //     break;

  //   case ROT_PB_EDIT_CTRL:
  //     read_button_dec_switch(BUTTON_MINUS, 0, 4, rotory_encoder.value, remoteIO);
  //     read_button_inc_switch(BUTTON_PLUS, 0, 4, rotory_encoder.value, remoteIO);
  //     control_mode = rotory_encoder.value;
  //     save_memory(5, 1, control_mode);
  //     break;

  //   case ROT_PB_EDIT_CH:
  //     read_button_dec_switch(BUTTON_MINUS, 0, 15, rotory_encoder.value, remoteIO);
  //     read_button_inc_switch(BUTTON_PLUS, 0, 15, rotory_encoder.value, remoteIO);
  //     radio_channel = rotory_encoder.value;
  //     save_memory(4, 1, radio_channel);
  //     break;

  // }
  // rotory_encoder.switch_value_old = rotory_encoder.switch_value;

  if (now - PrepareMessageTimer > 20)
  {
    PrepareMessageTimer = now;
    prepareOutMessage();    
  }

  if (now - DisplayUpdateTimer > 200)
  {
    DisplayUpdateTimer = now;
    display_refresh(radio_channel);    
  }


  if (now - SerialRawTimer > 500)
  {
    SerialRawTimer = now;
    // serialPrintTx(message_transmit);    
    // Serial.println("radio_connected: "+String(radio.isChipConnected()));
    // Serial.println(print_counter);
    print_counter++;
    print_io2();
    Serial.println();
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

void display_refresh(uint8_t radio_channel)
{
	display.setTextSize(1);
	display.setTextColor(WHITE);

	// LEFT COLUMN
	byte column = 0;
	byte line = 0;
	byte space = 0;

	display.setCursor(column, line);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_LX)
		display.setTextColor(BLACK, WHITE);
	display.print("LX : ");
	display.println(message_transmit.analog_left_X);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_LX)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_LY)
		display.setTextColor(BLACK, WHITE);
	display.print("LY : ");
	display.println(message_transmit.analog_left_Y);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_LY)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	display.print("LSW: ");
	display.println(analog_left_switch_state);

	line = line + 10;
	display.setCursor(column, line);
	display.print("SID: ");
	#ifdef PROMINI
		// display.println(digitalRead(SIDE_SWITCH));
	#endif
	#ifdef NODEMCU
		// display.println(remoteIO.digitalRead(SIDE_SWITCH));
	#endif


	line = line + 10;
	display.setCursor(column, line);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_CTRL)
		display.setTextColor(BLACK, WHITE);
	display.print("MOD: ");

	String output;
	switch (control_mode)
	{
    case CONTROLS_STANDARD:
      output = "STD";
      break;

    case CONTROLS_ENCHANCED:
      output = "ENCH";
      break;

    case CONTROLS_MEASURED:
      output = "MSRD";
      break;

    case CONTROLS_AUTONOMUS:
      output = "AUTO";
      break;

    default:
      output = "NONE";
	}
	display.println(output);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_CTRL)
		display.setTextColor(WHITE);

	// RIGHT COLUMN
	column = 64;
	line = 0;
	/*
	display.setTextColor(BLACK, WHITE);
	display.setTextColor(WHITE);
	*/
	display.setCursor(column, line);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_RX)
		display.setTextColor(BLACK, WHITE);
	display.print("RX : ");	
	display.println(message_transmit.analog_right_X);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_RX)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_RY)
		display.setTextColor(BLACK, WHITE);
	display.print("RY : ");	
	display.println(message_transmit.analog_right_Y);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_RY)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	display.print("RSW: ");
	display.println(analog_right_switch_state);

	line = line + 10;
	display.setCursor(column, line);
	display.print("POT: ");
	display.println(message_transmit.potentiometer);

	line = line + 10;
	display.setCursor(column, line);
	display.print("DIS: ");
	display.println(message_receive.distance);
	
  line = 47;
	column = 85;
  display.setCursor(column, line);
  display.setTextColor(WHITE);
  display.print(gui_menu.option);

  line = 47;
	column = 100;
  display.setCursor(column, line);
  display.setTextColor(WHITE);
  display.print(gui_menu.selected);

  line = 47;
	column = 115;
  display.setCursor(column, line);
  display.setTextColor(WHITE);
  display.print(gui_menu.value);



  for (int x = 5; x >= 0; x--)
  { 
    if (remoteIO1.digitalRead(x) == 0)
    {
      display.drawCircle(80- (x*10 + 10), 51, 3, WHITE);
    }

  }
  

	// BOTTOM BAR
	line = 57;
	column = 0;
	display.setCursor(column, line);
	display.print("Tx");

	column = column + 13;
	display.setCursor(column, line);
	display.print(message_counter);

	column = column + 22;
	display.setCursor(column, line);
	display.print("Rx");

	column = column + 13;
	display.setCursor(column, line);
	display.print(message_receive.message_no);

	column = column + 22;	
	display.setCursor(column, line);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_CH)
		display.setTextColor(BLACK, WHITE);
	display.print("CH");
	
	column = column + 13;	
	display.setCursor(column, line);
	display.print(radio_channel);
	if (rotory_encoder.switch_value == ROT_PB_EDIT_CH)
		display.setTextColor(WHITE);

	

	/*
	display.print("DEL");
	display.setCursor(92, line);
	display.println(message_receive.time_delay);
	*/

	display.display();
	display.clearDisplay();
}

void prepareOTA()
{
    #ifdef WIFI_STATION
    const char* ssid = "smarzony";
    const char* password = "metalisallwhatineed";
    WiFi.mode(WIFI_STA);
    #endif
    #ifdef WIFI_ACCESS_POINT
    const char* ssid = "RC Transmitter";
    const char* password = "smarzony";
    WiFi.softAP(ssid, password);
    #endif
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname("RC");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void menu()
{
  if (button_plus.rising() && gui_menu.selected == 0)
  {
    gui_menu.option++;
  }

  if (button_minus.rising() && gui_menu.selected == 0)
  {
    gui_menu.option--;
  }

  if (button_plus.rising() && gui_menu.selected != 0)
  {
    gui_menu.value++;
  }

  if (button_minus.rising() && gui_menu.selected != 0)
  {
    gui_menu.value--;
  }

  if (button_select.rising() && gui_menu.selected == 0)
  {
    gui_menu.selected = gui_menu.option;
  }

  else if (button_select.rising() && gui_menu.selected != 0)
  {
    gui_menu.selected = 0;
  }

  if (gui_menu.option < 0)
    {
      gui_menu.option = MENU_SIZE;
    }
  else if (gui_menu.option > MENU_SIZE)
    {
      gui_menu.option = 0;
    }
}
