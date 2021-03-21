#include "memory.h"

uint8_t get_memory(uint16_t address, uint8_t block_size)
{
	uint8_t value;
	uint16_t eeAddress = address * block_size;
	EEPROM.begin(128);
	delay(10);
	value = EEPROM.read(eeAddress);
	Serial.println("Readed value " + String(value) + " on address " + String(eeAddress));
	return value;
}

void save_memory(uint16_t address, uint8_t block_size, uint8_t value)
{
	uint16_t eeAddress = address * block_size;
	Serial.println("Saved value " + String(value) + " on address " + String(eeAddress));
	if (value <= 255)
		EEPROM.begin(128);
		delay(10);
		EEPROM.put(eeAddress, value);
		EEPROM.commit();
}