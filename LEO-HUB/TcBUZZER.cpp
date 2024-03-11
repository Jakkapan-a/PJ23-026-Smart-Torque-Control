
#include "TcBUZZER.h"

TcBUZZER::TcBUZZER(uint8_t pin, uint8_t active) // Default active low
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
        this->total--;
        // Active buzzer
        digitalWrite(this->pin, this->active);
        this->previousMillisDuty = currentMillis;
    }else if(currentMillis - this->previousMillisDuty > this->toneTime - (int)(this->dutyCycle * this->toneTime / 100))
    {   // Inactive buzzer
        digitalWrite(this->pin, !this->active);
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
  if(this->total > 0) return; // If the buzzer is already on, do nothing
  this->total = _total;
  this->previousMillisDuty = millis();
  digitalWrite(this->pin, this->active);
}

void TcBUZZER::off()
{
  this->total = 0;
  digitalWrite(this->pin, !this->active);
}

void TcBUZZER::setSequence(uint8_t _total, uint32_t _toneTime, uint8_t _dutyCycle)
{
  this->total = _total;
  this->toneTime = _toneTime;
  this->dutyCycle = _dutyCycle;
}