#include "pinstate.h"
#include "Arduino.h"
#include <stdint.h>

bool PinState::rising()
{
if (this-> actual == HIGH && this-> last == LOW)
    return true;
else
    return false;
}

bool PinState::falling()
{
if (this-> actual == LOW && this-> last == HIGH)
    return true;
else
    return false;
}
bool PinState::change()
{
if (this-> actual != this-> last) 
    return true;
else
    return false;
}