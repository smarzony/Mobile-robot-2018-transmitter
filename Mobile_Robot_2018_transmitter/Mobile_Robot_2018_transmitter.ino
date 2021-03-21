/*
Mobile robot transmitter code
Author: Piotr Smarzyński
*/

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

#define MENU_OPT_CALIBRATION_LX 1
#define MENU_OPT_CALIBRATION_LY 2
#define MENU_OPT_CALIBRATION_RX 3
#define MENU_OPT_CALIBRATION_RY 4
#define MENU_OPT_CTRL_MODE 5
#define MENU_OPT_CHANNEL 6
#define MENU_SIZE 6

#define EEPROM_CORRECTION_LX 0
#define EEPROM_CORRECTION_LY 1
#define EEPROM_CORRECTION_RX 2
#define EEPROM_CORRECTION_RY 3
#define EEPROM_CHANNEL 4
#define EEPROM_CONTROL_MODE 5

#define ROT_PB_EDIT_NONE 0
#define ROT_PB_EDIT_LX 1
#define ROT_PB_EDIT_LY 2
#define ROT_PB_EDIT_RX 3
#define ROT_PB_EDIT_RY 4
#define ROT_PB_EDIT_CTRL 5
#define ROT_PB_EDIT_CH 6
#define ROTORY_ENCODER_SWITCH_MAX 6


#define BUTTON_DELAY 300
#define ROTORY_ENCODER_CHANGE_MIN_TIME 50

//SERIAL OUTPUT

struct menu {
  int option;
  int selected;
  int value;
  bool just_selected;
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
  // Some magic to set bits in byte

  message_transmit.bit_array ^= (-analog_left_switch_state ^ message_transmit.bit_array) & (1UL << 1);
  message_transmit.bit_array ^= (-analog_right_switch_state ^ message_transmit.bit_array) & (1UL << 2);

  message_transmit.bit_array ^= (-remoteIO1.digitalRead(0) ^ message_transmit.bit_array) & (1UL << 3);
  message_transmit.bit_array ^= (-remoteIO1.digitalRead(0) ^ message_transmit.bit_array) & (1UL << 4);
  message_transmit.bit_array ^= (-remoteIO1.digitalRead(0) ^ message_transmit.bit_array) & (1UL << 5);
  message_transmit.bit_array ^= (-remoteIO1.digitalRead(0) ^ message_transmit.bit_array) & (1UL << 6);
  message_transmit.bit_array ^= (-remoteIO1.digitalRead(0) ^ message_transmit.bit_array) & (1UL << 7);
  message_transmit.bit_array ^= (-remoteIO1.digitalRead(0) ^ message_transmit.bit_array) & (1UL << 8);



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

void readRadio()
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
    analog_correction.analog_left_X_correct = int(get_memory(EEPROM_CORRECTION_LX, 1)) - 128;
    analog_correction.analog_left_Y_correct = int(get_memory(EEPROM_CORRECTION_LY, 1)) - 128;
    analog_correction.analog_right_X_correct = int(get_memory(EEPROM_CORRECTION_RX, 1)) - 128;
    analog_correction.analog_right_Y_correct = int(get_memory(EEPROM_CORRECTION_RY, 1)) - 128;
  #endif

  //---------------------- Radio config BEGIN -----------------

  Serial.begin(115200);
  radio_channel = get_memory(EEPROM_CHANNEL, 1);
  if (radio_channel > 120)
    {
      radio_channel = 0;
    }
  control_mode = get_memory(EEPROM_CONTROL_MODE, 1);
  if (control_mode > CONTROLS_AUTONOMUS)
  {
    control_mode = CONTROLS_NONE;
  }

  radio.begin();
  radio.setDataRate(RF24_1MBPS);
  radio.setRetries(2, 5);
  radio.setChannel(radio_channel);// 100
  // -----------   JAK SIE ROZJEBIE TO ZMIEN KANAL ---------
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

  button_minus.last = button_minus.actual;
  button_select.last = button_select.actual;
  button_plus.last = button_plus.actual;
  button_minus.actual = remoteIO.digitalRead(button_minus.no);
  button_select.actual = remoteIO.digitalRead(button_select.no);
  button_plus.actual = remoteIO.digitalRead(button_plus.no); 
  read_button_neg_switch(ANALOG_LEFT_PUSHBUTTON, analog_left_switch_state, remoteIO1);
  read_button_neg_switch(ANALOG_RIGHT_PUSHBUTTON, analog_right_switch_state, remoteIO1);  

  menu();
  readRadio();

  if (now - PrepareMessageTimer > 20)
  {
    PrepareMessageTimer = now;
    prepareOutMessage();
    analog_left_switch_state = false;
    analog_right_switch_state = false;
  }

  if (now - DisplayUpdateTimer > 200)
  {
    DisplayUpdateTimer = now;
    display_refresh(radio_channel);    
  }

  if (now - SerialRawTimer > 500)
  {
    SerialRawTimer = now;
    print_io2();
    Serial.println();
  }
}

void display_refresh(uint8_t radio_channel)
{
	display.setTextSize(1);
	display.setTextColor(WHITE);

	// LEFT COLUMN
	byte column = 0;
	byte line = 0;

	display.setCursor(column, line);
	if (gui_menu.option == MENU_OPT_CALIBRATION_LX)
		display.setTextColor(BLACK, WHITE);
	display.print("LX : ");
	display.println(message_transmit.analog_left_X);
	if (gui_menu.option == MENU_OPT_CALIBRATION_LX)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	if (gui_menu.option == MENU_OPT_CALIBRATION_LY)
		display.setTextColor(BLACK, WHITE);
	display.print("LY : ");
	display.println(message_transmit.analog_left_Y);
	if (gui_menu.option == MENU_OPT_CALIBRATION_LY)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	if (gui_menu.option == MENU_OPT_CTRL_MODE)
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
	if (gui_menu.option == MENU_OPT_CTRL_MODE)
		display.setTextColor(WHITE);

  for (int x = 5; x >= 0; x--)
  { 
    if (remoteIO1.digitalRead(x) == 0)
    {
      display.drawCircle(61- (x*10 + 7), 34, 3, WHITE);
    }

  }

	// RIGHT COLUMN
	column = 64;
	line = 0;

	display.setCursor(column, line);
	if (gui_menu.option == MENU_OPT_CALIBRATION_RX)
		display.setTextColor(BLACK, WHITE);
	display.print("RX : ");	
	display.println(message_transmit.analog_right_X);
	if (gui_menu.option == MENU_OPT_CALIBRATION_RX)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	if (gui_menu.option == MENU_OPT_CALIBRATION_RY)
		display.setTextColor(BLACK, WHITE);
	display.print("RY : ");	
	display.println(message_transmit.analog_right_Y);
	if (gui_menu.option == MENU_OPT_CALIBRATION_RY)
		display.setTextColor(WHITE);

	line = line + 10;
	display.setCursor(column, line);
	display.print("POT: ");
	display.println(message_transmit.potentiometer);

	line = line + 10;
	display.setCursor(column, line);
	display.print("DIS: ");
	display.println(message_receive.distance);
  
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
	if (gui_menu.option == MENU_OPT_CHANNEL)
		display.setTextColor(BLACK, WHITE);
	display.print("CH");
	
	column = column + 13;	
	display.setCursor(column, line);
	display.print(radio_channel);
	if (gui_menu.option == MENU_OPT_CHANNEL)
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
  gui_menu.just_selected = false;
  if (gui_menu.selected == 0)
  {
    if (button_plus.rising())   // scroll plus through options
    {
      gui_menu.option++;
    }
    if (button_minus.rising())  // scroll minus through options
    {
      gui_menu.option--;
    }

    if (button_select.rising())   // get into option
    {
      gui_menu.just_selected = true;
      gui_menu.selected = gui_menu.option;
    }

    if (gui_menu.option < 0)  // makse sure that you are in available options
    {
      gui_menu.option = MENU_SIZE;
    }
    else if (gui_menu.option > MENU_SIZE)
    {
      gui_menu.option = 0;
    }
  }

  if (gui_menu.selected == MENU_OPT_CALIBRATION_LX) // option 1 - calibrate correction on left X axis
  {    
    if (gui_menu.just_selected == true)
    {
      gui_menu.value = analog_correction.analog_left_X_correct;
    }
    if (button_plus.rising())
    {
      analog_correction.analog_left_X_correct++;
    }

    if (button_minus.rising())
    {
      analog_correction.analog_left_X_correct--;
    }  

    if (button_select.rising() and gui_menu.just_selected == false)
    {
      save_memory(EEPROM_CORRECTION_LX, 1, analog_correction.analog_left_X_correct + 128);
      gui_menu.selected = 0;
      gui_menu.value = 0;
    } 
  }

  if (gui_menu.selected == MENU_OPT_CALIBRATION_LY) // option 2 - calibrate correction on left Y axis
  {    
    if (gui_menu.just_selected == true)
    {
      gui_menu.value = analog_correction.analog_left_Y_correct;
    }
    if (button_plus.rising())
    {
      analog_correction.analog_left_Y_correct++;
    }

    if (button_minus.rising())
    {
      analog_correction.analog_left_Y_correct--;
    }  

    if (button_select.rising() and gui_menu.just_selected == false)
    {

      save_memory(EEPROM_CORRECTION_LY, 1, analog_correction.analog_left_Y_correct + 128);
      gui_menu.selected = 0;
      gui_menu.value = 0;
    } 
  }

  if (gui_menu.selected == MENU_OPT_CALIBRATION_RX) // option 3 - calibrate correction on right X axis
  {    
    if (gui_menu.just_selected == true)
    {
      gui_menu.value = analog_correction.analog_right_X_correct;
    }
    if (button_plus.rising())
    {
      analog_correction.analog_right_X_correct++;
    }

    if (button_minus.rising())
    {
      analog_correction.analog_right_X_correct--;
    }  

    if (button_select.rising() and gui_menu.just_selected == false)
    {
      save_memory(EEPROM_CORRECTION_RX, 1, analog_correction.analog_right_X_correct + 128);
      gui_menu.selected = 0;
      gui_menu.value = 0;
    } 
  }

  if (gui_menu.selected == MENU_OPT_CALIBRATION_RY) // option 4 - calibrate correction on right Y axis
  {    
    if (gui_menu.just_selected == true)
    {
      gui_menu.value = analog_correction.analog_right_Y_correct;
    }
    if (button_plus.rising())
    {
      analog_correction.analog_right_Y_correct++;
    }

    if (button_minus.rising())
    {
      analog_correction.analog_right_Y_correct--;
    }  

    if (button_select.rising() and gui_menu.just_selected == false)
    {
      save_memory(EEPROM_CORRECTION_RY, 1, analog_correction.analog_right_Y_correct + 128);
      gui_menu.selected = 0;
      gui_menu.value = 0;
    } 
  }

  if (gui_menu.selected == MENU_OPT_CTRL_MODE) // option 5 - set control mode
  {
    if (button_plus.rising())
    {
      if (control_mode <= CONTROLS_AUTONOMUS)
      {
       control_mode++;
      }
      else
      {
        control_mode = CONTROLS_NONE;
      }
    }

    if (button_minus.rising())
    {
      if (control_mode >= CONTROLS_NONE)
      {
       control_mode--;
      }
      else
      {
        control_mode = CONTROLS_AUTONOMUS;
      }
    }  

    if (button_select.rising() and gui_menu.just_selected == false)
    {
      save_memory(EEPROM_CONTROL_MODE, 1, control_mode);
      gui_menu.selected = 0;
      gui_menu.value = 0;
    } 
  }

if (gui_menu.selected == MENU_OPT_CHANNEL) // option 5 - set channel
  {
    if (button_plus.rising())
    {
      if (radio_channel <= 120)
      {
       radio_channel++;
      }
      else
      {
        radio_channel = 0;
      }
    }

    if (button_minus.rising())
    {
      if (radio_channel >= 0)
      {
       radio_channel--;
      }
      else
      {
        radio_channel = 120;
      }
    }  

    if (button_select.rising() and gui_menu.just_selected == false)
    {
      save_memory(EEPROM_CHANNEL, 1, radio_channel);
      gui_menu.selected = 0;
      gui_menu.value = 0;
    } 
  }
}

  

