#include <SoftwareSerial.h>
#define RX_PIN 10
#define TX_PIN 11
SoftwareSerial rs485(RX_PIN, TX_PIN); // RX, TX
void setup()
{
    rs485.begin(9600);
    Serial.begin(9600);
}

void loop()
{
    //Serial.println("Hello, world!");
    // rs485.println("Hello, world!");
    //delay(20);
    if(rs485.available())
    {
        String data = rs485.readString();
        Serial.println(data);
    }

    //delay(2000);
}
