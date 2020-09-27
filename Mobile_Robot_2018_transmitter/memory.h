#include "Arduino.h"
#include <EEPROM.h>

uint8_t get_memory(uint16_t address, uint8_t block_size);
void save_memory(uint16_t address, uint8_t block_size, uint8_t value);
