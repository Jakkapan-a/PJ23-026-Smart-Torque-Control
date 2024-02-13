#include <TcBUTTON.h>
#include <TcPINOUT.h>

// ------------------  VARIABLES  ------------------ //
bool startReceived = false;
bool endReceived = false;

const byte startChar = 0x02;
const byte endChar = 0x03;
String inputString = "";


void serialEvent3() {
  while (Serial3.available()) {
    byte inChar = (byte)Serial3.read();
    if (inChar == startChar) {
      startReceived = true;
      inputString = "";

      inputString += (char)inChar;
    } else if (inChar == endChar) {
      inputString += (char)inChar;

      endReceived = true;
    } else {
      inputString += (char)inChar;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial3.begin(9600);
  Serial.println("Start............");
}

void loop() {
  if (startReceived && endReceived) {
    // Serial.println(inputString);
    for (int i = 0; i < inputString.length(); i++) {
      Serial.print(inputString[i], DEC);
      Serial.print(" ");
    }
    Serial.println("------------------");

    startReceived = false;
    endReceived = false;
    inputString = "";
  }
}

// void ToneFun(unsigned long _toneTime, int _toneFreq, int _tonePercent, int &totalTone) {
//   if (totalTone <= 0) {
//     return;
//   }
//   unsigned long currentMillis = millis();
//   if (currentMillis - lastTimeTone > _toneTime) {
//     if (_tonePercent > 0) {
//       int p = (int)(_tonePercent * _toneTime / 100);
//       tone(BUZZER_PIN, _toneFreq, p);
//       totalTone--;
//     }
//     lastTimeTone = currentMillis;
//   } else if (currentMillis < lastTimeTone) {
//     lastTimeTone = currentMillis;
//   }
// }
