#include <TcBUTTON.h>
#include <TcPINOUT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
// -------------------- LCD -------------------- //
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- INPUT -------------------- //
#define BTN_RUN 11
void btnRunOnEventChange(bool state);
TcBUTTON btnRun(BTN_RUN,false);
#define BTN_STOP 12
void btnStopOnEventChange(bool state);
TcBUTTON btnStop(BTN_STOP,false);

#define BTN_ESC_PIN 35
void btnEscOnEventChange(bool state);
TcBUTTON btnEsc(BTN_ESC_PIN,false);

#define BTN_UP_PIN 36
void btnUpOnEventChange(bool state);
TcBUTTON btnUp(BTN_UP_PIN,false);

#define BTN_DOWN_PIN 37
void btnDownOnEventChange(bool state);
TcBUTTON btnDown(BTN_DOWN_PIN,false);

#define BTN_ENTER_PIN 38
void btnEnterOnEventChange(bool state);
TcBUTTON btnEnter(BTN_ENTER_PIN,false);

#define BTN_CENSOR_ON_ST 40
void btnCensorOnStOnEventChange(bool state);
TcBUTTON btnCensorOnSt(BTN_CENSOR_ON_ST,false);
// -------------------- OUTPUT -------------------- //
#define LED_BLUE 7
TcPINOUT ledBlue(LED_BLUE,false);
#define LED_GREEN 8
TcPINOUT ledGreen(LED_GREEN,false);
#define LED_RED 9
TcPINOUT ledRed(LED_RED,false);

enum STATUS_TEST
{
  PASS,
  NG,
  TESTING,
  NO_TEST,
  STOP
};

STATUS_TEST status_test = NO_TEST;
#define BUZZER_PIN 2

// -------------------------- Variables -------------------------- //
// Define an array containing letters, numbers, and some symbols
const char letters[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                         'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                         'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                         'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                         '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                         '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_', '=', '+' };
int numChars = sizeof(letters) - 1;
int indexChar = 0;


unsigned long lastTimeTone = 0;

void setup() {
  Serial.begin(115200);
  btnRun.OnEventChange(btnRunOnEventChange);
  btnRun.DebounceDelay(20);
  btnStop.OnEventChange(btnStopOnEventChange);
  btnStop.DebounceDelay(20);

  btnEsc.OnEventChange(btnEscOnEventChange);
  btnUp.OnEventChange(btnUpOnEventChange);
  btnDown.OnEventChange(btnDownOnEventChange);
  btnEnter.OnEventChange(btnEnterOnEventChange);
  btnCensorOnSt.OnEventChange(btnCensorOnStOnEventChange);
}

void loop() {

}

void ToneFun(unsigned long _toneTime, int _toneFreq, int _tonePercent, int &totalTone) {
  if (totalTone <= 0) {
    return;
  }
  unsigned long currentMillis = millis();
  if (currentMillis - lastTimeTone > _toneTime) {
    if (_tonePercent > 0) {
      int p = (int)(_tonePercent * _toneTime / 100);
      tone(BUZZER_PIN, _toneFreq, p);
      totalTone--;
    }
    lastTimeTone = currentMillis;
  } else if (currentMillis < lastTimeTone) {
    lastTimeTone = currentMillis;
  }
}
