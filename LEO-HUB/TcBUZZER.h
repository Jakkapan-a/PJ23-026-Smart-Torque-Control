#ifndef Tc_BUZZER_h
#define Tc_BUZZER_h

#include "Arduino.h"

class TcBUZZER
{
 protected:
	uint8_t pin = 0;
    bool active = 1;
    bool state = false;
    // uint32_t previousMillis;
    uint32_t previousMillisDuty;
    uint32_t toneTime = 500; // 500ms by default
    uint8_t dutyCycle = 50; // 50% duty cycle by default
    
    public:
    enum BUZZER_ACTIVE
    {
       ACTIVE_HIGH = 1,
       ACTIVE_LOW = 0
    };

    uint8_t total = 0;
    TcBUZZER(uint8_t pin, bool active);
    void begin();
    void update();
    void on(uint8_t _total);
    void off();
    void setTime(uint32_t _toneTime);
    void setDutyCycle(uint8_t _dutyCycle);
};

#endif