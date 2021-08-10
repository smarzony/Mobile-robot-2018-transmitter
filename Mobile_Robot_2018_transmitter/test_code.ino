#include "radio_data.h"
void print_io2()
{
    // Serial.println();
    for(int i = 0; i <= 7; i++)
    {
        Serial.println("I" + String(i) + ": " + String(remoteIO.digitalRead(i)) + " " + String(remoteIO1.digitalRead(i)));
    }
}

void print_rx_message()
{ 
    Serial.println(message_receive.distance);
    Serial.println(message_receive.velocity_measured_left);
    Serial.println(message_receive.velocity_measured_right);
    Serial.println(message_receive.time_delay);
    Serial.println(message_receive.message_no);
}

void print_rx_coords(double pos_long, double pos_lat)
{ 

    Serial.print(pos_long);
    Serial.print("\t");
    Serial.println(pos_lat);
}