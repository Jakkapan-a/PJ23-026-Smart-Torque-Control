
#include "TcBUZZER.h"

TcBUZZER::TcBUZZER(uint8_t pin, bool active) // Default active low
{
  this->pin = pin;
  this->active = active;
  this->begin();
}

void TcBUZZER::begin()
{
  pinMode(this->pin, OUTPUT);
  this->off();
}

void TcBUZZER::update()
{
  if (this->total > 0)
  {
    uint32_t currentMillis = millis();
    if(currentMillis - this->previousMillisDuty > this->toneTime)
    {
        // Active buzzer
        digitalWrite(this->pin, this->active);
        this->previousMillisDuty = currentMillis;
        this->state = this->active;
    }else if(this->state == this->active &&currentMillis - this->previousMillisDuty > this->toneTime - (int)(this->dutyCycle * this->toneTime / 100))
    {   // Inactive buzzer
        digitalWrite(this->pin, !this->active);
        this->total--;
        this->state = !this->active;
    }
    else if(currentMillis < this->previousMillisDuty)
    {
      this->previousMillisDuty = currentMillis;
    }
  }
  else
  {
    this->off();
  }
}

void TcBUZZER::on(uint8_t _total)
{
  this->total = _total;
  if(this->total > 0) return; // If the buzzer is already on, do nothing
  this->previousMillisDuty = millis();
  digitalWrite(this->pin, this->active);
}

void TcBUZZER::off()
{
//   if(this->total == 0) return; // If the buzzer is already off, do nothing
  this->total = 0;
  digitalWrite(this->pin, !this->active);
}

void TcBUZZER::setTime(uint32_t _toneTime)
{
  this->toneTime = _toneTime;
}

void TcBUZZER::setDutyCycle(uint8_t _dutyCycle)
{
  this->dutyCycle = _dutyCycle;
}