void print_io2()
{
    // Serial.println();
    for(int i = 0; i <= 7; i++)
    {
        Serial.println("I" + String(i) + ": " + String(remoteIO.digitalRead(i)) + " " + String(remoteIO1.digitalRead(i)));
    }
}