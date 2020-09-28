#pragma once
#include "Arduino.h"
#include "radio_data.h"
#include <jm_PCF8574.h>


void serialPrintTx(radioDataTrasnsmit &message_transmit);
void serialPrintPCF(jm_PCF8574 &remoteIO);