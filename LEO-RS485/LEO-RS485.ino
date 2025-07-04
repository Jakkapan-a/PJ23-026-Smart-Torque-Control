/**
 * @file LEO-HUB.ino
 * @author Jakkapan
 * @brief
 * @version 1.0 ModbusRTU
 * @date 2024-03-05
 * Arduino leonardo board
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <TcBUTTON.h>
#include <TcPINOUT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "TcBUZZER.h"
#include <ModbusRTUSlave.h>

// -------------------- PIN -------------------- //
#define BTN_CENSOR_ON_ST 7
void btnCensorOnStOnEventChange(bool state);
// TcBUTTON btnCensorOnSt(BTN_CENSOR_ON_ST, true);
TcBUTTON btnCensorOnSt(BTN_CENSOR_ON_ST);

#define START_PIN 8
void startOnEventChange(bool state);
// TcBUTTON startButton(START_PIN, false);
TcBUTTON startButton(START_PIN);

#define STOP_PIN 9
void stopOnEventChange(bool state);
// TcBUTTON stopButton(STOP_PIN, false);
TcBUTTON stopButton(STOP_PIN);

#define END_PIN 12
void endOnEventChange(bool state);
// TcBUTTON endButton(END_PIN, true);
 TcBUTTON endButton(END_PIN);

// -------------------- RELAY -------------------- //
#define LED_RED_PIN 4
TcPINOUT ledRed(LED_RED_PIN);

#define LED_GREEN_PIN 5
TcPINOUT ledGreen(LED_GREEN_PIN);

#define LED_BLUE_PIN 13
TcPINOUT ledBlue(LED_BLUE_PIN);

#define RELAY_RED_PIN A0
TcPINOUT relayRed(RELAY_RED_PIN);

#define RELAY_GREEN_PIN A1
TcPINOUT relayGreen(RELAY_GREEN_PIN);

#define RELAY_BLUE_PIN A2
TcPINOUT relayBlue(RELAY_BLUE_PIN);

#define RELAY_ALARM_PIN A3
TcPINOUT relayAram(RELAY_ALARM_PIN);

#define RELAY_LOCK_JIG_PIN A4
TcPINOUT relayLockJig(RELAY_LOCK_JIG_PIN);

#define RELAY_TORQUE_PWR_PIN A5
TcPINOUT relayTorquePwr(RELAY_TORQUE_PWR_PIN);

#define BUZZER_PIN 6
TcBUZZER buzzerPass(BUZZER_PIN, true);

#define RX_PIN 10
#define TX_PIN 11
SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX

const uint8_t slaveID = 1;

ModbusRTUSlave modbus(mySerial);
bool coils[20] = { false };
bool discreteInputs[20] = { false };

uint16_t holdingRegisters[30] = { 0 };
char _cardNumber[17] = "                ";
// -------------------- LCD -------------------- //
LiquidCrystal_I2C lcd(0x27, 16, 2);
void LED_Controls(uint8_t);
void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);
  mySerial.begin(9600);

  lcd.begin();
  lcd.clear();
  lcd.backlight();

  modbus.begin(slaveID, 9600);
  modbus.configureCoils(coils, 20);
  modbus.configureDiscreteInputs(discreteInputs, 20);
  modbus.configureHoldingRegisters(holdingRegisters, 30);
  
  btnCensorOnSt.isInvert = true;
  btnCensorOnSt.setOnEventChange(btnCensorOnStOnEventChange);
  
  startButton.setOnEventChange(startOnEventChange);
  startButton.setDebounceDelay(20);  // Set debounce delay to 20ms

  stopButton.setOnEventChange(stopOnEventChange);
  stopButton.setDebounceDelay(20);  // Set debounce delay to 20ms

  endButton.isInvert = true;
  endButton.setOnEventChange(endOnEventChange);
  endButton.setDebounceDelay(10);  // Set debounce delay to 500us
  buzzerPass.setTime(300);

  Serial.println("Start");
  coils[2] = true;
  // Update LCD to starting...
  updateLCD("Starting...", "Please wait...");
  LED_Controls(0);
}
int count = 0;
uint8_t oldStateType = 0;
uint32_t LastTime200 = 0;
uint32_t LastTime100 = 0;
uint32_t TimeStampStart = 0;
uint32_t TimeStampStop = 0;
int delayProcess = 0;
uint32_t _timeDiff = 0;
int delayStopOff = 0;
int delayEndOff = 0;
int delayLogJig = 0;
void loop() {
  modbus.poll();
  
  btnCensorOnSt.update();
  startButton.update();
  stopButton.update();
  endButton.update();
  buzzerPass.update();

  count++;
  holdingRegisters[0] = count;
  if (count >= 1000) {
    count = 0;
  }
  ReadRFID();
  if (coils[1] == false && discreteInputs[1] == true) {
    // Clear card data
    holdingRegisters[1] = 0;
    holdingRegisters[2] = 0;
    holdingRegisters[3] = 0;

    Serial.println("Card data cleared");

    coils[1] = false;
    discreteInputs[1] = false;
  }

  if (coils[2] == true) {
    discreteInputs[2] = true;
    return;
  } else if (coils[2] == false && discreteInputs[2] == true) {
    discreteInputs[2] = false;
    buzzerPass.total += 2;
    buzzerPass.setTime(300);
    // Init Data for process
  }
  // ---------- Update Next Process -----------
  if (coils[11] == true) {
    coils[11] = false;
    _timeDiff = 0;
    TimeStampStart = 0;
    // -----------------

    if (discreteInputs[7] == true && holdingRegisters[18] < holdingRegisters[9]) {
      relayTorquePwr.on();
      discreteInputs[9] = false;
    } else {
      relayTorquePwr.off();
    }
  }

  // ----------------- RFID
  if (coils[9] == true) {
    coils[9] = false;
    // Update LCD to Accept or Not Accept
    _timeDiff = 0;
    String line1 = "Accept or Not Accept";
    String line2 = "UUID:" + String(_cardNumber);
    if (coils[4] == true) {
      coils[10] = false;
      line1 = "Accept";
      discreteInputs[7] = false;
      buzzerPass.total = 1;
      buzzerPass.setTime(300);
      // LED OFF
      LED_Controls(0);

      relayLockJig.off();
    } else {
      line1 = "Not Accept";
      buzzerPass.setTime(300);
      buzzerPass.total += 5;
    }
    updateLCD(line1, line2);
    delayProcess = 20;
    return;
  }

  // ----------------- SUM CHECK IS NG ----------------- //
  if (coils[10] == true) {
    buzzerPass.setTime(300);
    buzzerPass.total = 15;
    // Pwr off
    relayTorquePwr.off();

    // LED RED
    LED_Controls(1);
    return;
  }
  uint32_t currentMillis = millis();

  // ----------------- 100ms ----------------- //
  if (currentMillis - LastTime100 >= 100) {
    LastTime100 = currentMillis;
    if (delayStopOff > 0) {
      delayStopOff--;
      if (delayStopOff == 0) {
        discreteInputs[6] = false;
      }
    }

    if (delayEndOff > 0) {
      delayEndOff--;
      if (delayEndOff == 0) {
        discreteInputs[8] = false;
      }
    }

    if (delayLogJig > 0) {
      delayLogJig--;
      if (delayLogJig == 0) {
        relayLockJig.on();
      }
    }

    if (delayProcess > 0) {
      delayProcess--;
      return;
    }
  } else if (currentMillis < LastTime100) {
    // Overflow
    LastTime100 = currentMillis;
  }

  if (delayProcess > 0) {
    return;
  }


  // ---------- SUM CHECK IS OK --------------
  if (coils[12] == true) {
    coils[12] = false;
    buzzerPass.setTime(300);
    buzzerPass.total = 2;
    relayLockJig.off();
  }

  if (discreteInputs[6] == false && discreteInputs[9] == true) {
    discreteInputs[9] = false;
    relayTorquePwr.off();
  }

  // ----------------- 200ms ----------------- //
  if (currentMillis - LastTime200 >= 200) {
    LastTime200 = currentMillis;

    String name = GetName();
    uint32_t timeDiff = TimeStampStop - TimeStampStart;
    if (TimeStampStop >= TimeStampStart) {
      timeDiff = TimeStampStop - TimeStampStart;
    }

    String line1 = name + ":" + String(holdingRegisters[7]) + "-" + String(holdingRegisters[8]);
    String line2 = "SCW:" + String(holdingRegisters[18]) + "/" + String(holdingRegisters[9]) + ":" + String(_timeDiff) + "ms";
    // ------------- Init -----------------//
    if (discreteInputs[7] == false) {
      String line2 = "Waiting....";
      // SET DISPLAY
      updateLCD(line1, line2);
      relayTorquePwr.off();
      return;
    }
    if (discreteInputs[5] == true && discreteInputs[6] == false) {
      timeDiff = currentMillis - TimeStampStart;
      line1 = name + ":" + String(holdingRegisters[7]) + "-" + String(holdingRegisters[8]);
      line2 = "SCW:" + String(holdingRegisters[18]) + "/" + String(holdingRegisters[9]) + ":" + String(timeDiff) + "ms";
    }


    else if (discreteInputs[5] == true && discreteInputs[6] == true) {
      line1 = name + ":" + String(holdingRegisters[7]) + "-" + String(holdingRegisters[8]);
      line2 = "SCW:" + String(holdingRegisters[18]) + "/" + String(holdingRegisters[9]) + ":" + String(timeDiff) + "ms";
    }

    // SET DISPLAY
    updateLCD(line1, line2);
  } else if (currentMillis < LastTime200) {
    // Overflow
    LastTime200 = currentMillis;
  }
}

void ReadRFID() {
  // Non-blocking RFID data reading
  if (Serial1.available() >= 9) {  // Ensure there are at least 9 bytes to read
    if (Serial1.read() == 0x02) {
      // Start of data
      byte length = Serial1.read();
      if (length == 0x09) {
        byte cardType = Serial1.read();
        byte cardData[4];
        for (int i = 0; i < 4; i++) {
          cardData[i] = Serial1.read();
        }
        byte bcc = Serial1.read();
        byte end = Serial1.read();

        if (end == 0x03) {  // End of data
          // Verify BCC
          byte bccCheck = length ^ cardType ^ cardData[0] ^ cardData[1] ^ cardData[2] ^ cardData[3];
          if (bcc == bccCheck) {

            Serial.print("Card Type: ");
            Serial.println(cardType == 0x02 ? "EM4100" : "Mifare 1K");

            // Convert card data to a single integer
            uint32_t cardNumber = (uint32_t)cardData[0] << 24 | (uint32_t)cardData[1] << 16 | (uint32_t)cardData[2] << 8 | cardData[3];

            Serial.print("Card Data: ");
            Serial.println(cardNumber);  // Print card number in decimal

            // Covert to string
            String cardNumberStr = String(cardNumber);
            cardNumberStr.toCharArray(_cardNumber, 17);
            // Store the card number into holding registers
            holdingRegisters[1] = (uint16_t)(cardNumber >> 16);     // Upper 16 bits
            holdingRegisters[2] = (uint16_t)(cardNumber & 0xFFFF);  // Lower 16 bits
            holdingRegisters[3] = cardType;                         // Card type (0x02 for EM4100, 0x03 for Mifare 1K)

            coils[1] = true;
            discreteInputs[1] = true;

            coils[3] = false;
            coils[4] = false;
          } else {
            Serial.println("BCC check failed");
          }
        } else {
          Serial.println("End byte mismatch");
        }
      } else {
        Serial.println("Incorrect length");
      }
    } else {
      Serial.println("Start byte mismatch");
    }
  }
}
void btnCensorOnStOnEventChange(bool state) {
  discreteInputs[7] = !state;
  if (discreteInputs[7]) {
    buzzerPass.total = 1;
    buzzerPass.setTime(300);
    // LED OFF
    LED_Controls(0);

    delayLogJig = 20;
  }

  // Check if the start button is pressed
  if (discreteInputs[7] == true) {
    relayTorquePwr.on();
  } else {
    relayTorquePwr.off();
  }
  coils[7] = true;
}

void startOnEventChange(bool state) {
  discreteInputs[5] = !state;

  if (discreteInputs[5] == true) {
    TimeStampStart = millis();

    holdingRegisters[20] = (uint16_t)(TimeStampStart >> 16);     // Upper 16 bits
    holdingRegisters[21] = (uint16_t)(TimeStampStart & 0xFFFF);  // Lower 16 bits

    Serial.print("Time Start: ");
    Serial.println(TimeStampStart);
    // LED BLUE
    LED_Controls(3);
  } else {
    // LED OFF
    if(holdingRegisters[18] != holdingRegisters[9]){
      LED_Controls(0);
    }
  }

  coils[5] = true;
}

void stopOnEventChange(bool state) {
  if (!state == false) {
    delayStopOff = 5;
    return;
  }
  discreteInputs[6] = !state;

  if (discreteInputs[6]) {
    //  millis();
    TimeStampStop = millis();
    buzzerPass.total = 1;
    buzzerPass.setTime(300);
    // Process time compare
    _timeDiff = TimeStampStop - TimeStampStart;

    Serial.print("Time Diff: ");
    Serial.println(_timeDiff);

    holdingRegisters[14] = (uint16_t)(TimeStampStop >> 16);     // Upper 16 bits
    holdingRegisters[15] = (uint16_t)(TimeStampStop & 0xFFFF);  // Lower 16 bits

    Serial.print("Time STOP: ");
    Serial.println(TimeStampStop);

    holdingRegisters[16] = (uint16_t)(_timeDiff >> 16);     // Upper 16 bits
    holdingRegisters[17] = (uint16_t)(_timeDiff & 0xFFFF);  // Lower 16 bits

    holdingRegisters[18]++;

    discreteInputs[6] = true;
    if (_timeDiff > holdingRegisters[7] && _timeDiff < holdingRegisters[8]) {
      // LED GREEN
      coils[10] = false;
      if (holdingRegisters[18] >= holdingRegisters[9]) {
        discreteInputs[9] = true;
      }
      LED_Controls(2);
    } else {
      // LED RED IS NG
      coils[10] = true;
      LED_Controls(1);
    }

    // Update display
    String name = GetName();

    String line1 = name + ":" + String(holdingRegisters[7]) + "-" + String(holdingRegisters[8]);
    String line2 = "SCW:" + String(holdingRegisters[18]) + "/" + String(holdingRegisters[9]) + ":" + String(_timeDiff) + "ms";
    // SET DISPLAY
    updateLCD(line1, line2);
  }

  coils[6] = true;
}

void endOnEventChange(bool state) {
  // IF CURRNET IS NG
  if (coils[10] == true) {
    return;
  }
  if (!state == false) {
    delayEndOff = 2;
    return;
  }
  discreteInputs[8] = !state;
  if (discreteInputs[8]) {
    // buzzerPass.total = 1;
    // buzzerPass.setTime(300);
    // Finish process
    discreteInputs[7] = false;
    // Validate
    if (holdingRegisters[18] != holdingRegisters[9]) {
      coils[10] == true;
    }
    // Get and clear data
    // LED OFF
    LED_Controls(0);
  }
  
  holdingRegisters[18] = 0;
  coils[8] = true;
}

char currentLine1[17] = "                ";  // 16 characters + null terminator
char currentLine2[17] = "                ";  // 16 characters + null terminator

void updateLCD(const String newDataLine1, const String newDataLine2) {
  updateLCD(newDataLine1.c_str(), newDataLine2.c_str());
}

void updateLCD(const char *newDataLine1, const char *newDataLine2) {
  updateLCDLine(newDataLine1, currentLine1, 0);
  updateLCDLine(newDataLine2, currentLine2, 1);
}

void updateLCDLine(const char *newData, char (&currentLine)[17], int row) {
  int i;
  // Update characters as long as they are different or until newData ends
  for (i = 0; i < 16 && newData[i]; i++) {
    if (newData[i] != currentLine[i]) {
      lcd.setCursor(i, row);
      lcd.print(newData[i]);
      currentLine[i] = newData[i];
    }
  }
  // Clear any remaining characters from the previous display
  for (; i < 16; i++) {
    if (currentLine[i] != ' ') {
      lcd.setCursor(i, row);
      lcd.print(' ');
      currentLine[i] = ' ';
    }
  }
}

String GetName() {
  char name[9];  // 8 characters + null terminator
  int index = 0;

  for (int i = 10; i <= 13; i++) {
    name[index++] = holdingRegisters[i] >> 8;    // Read upper byte
    name[index++] = holdingRegisters[i] & 0xFF;  // Read lower byte
  }

  name[8] = '\0';  // Null terminate string
  return String(name);
}

void LED_Controls(uint8_t state = 0) {
  // 1 = RED
  // 2 = GREEN
  // 3 = BLUE
  // 0 = OFF
  if (state == 0) {
    ledRed.off();
    ledGreen.off();
    ledBlue.off();

    relayGreen.off();
    relayBlue.on();
    relayRed.off();
    relayAram.off();

    // Serial.println("LED OFF");
  } else if (state == 1) {
    ledRed.on();
    ledGreen.off();
    ledBlue.off();

    relayGreen.off();
    relayBlue.off();
    relayRed.on();
    relayAram.on();

    // Serial.println("LED RED");
  } else if (state == 2) {
    ledRed.off();
    ledGreen.on();
    ledBlue.off();

    relayGreen.on();
    relayBlue.off();
    relayRed.off();

    // Serial.println("LED GREEN");
  } else if (state == 3) {
    ledRed.off();
    ledGreen.off();
    ledBlue.on();

    // Serial.println("LED BLUE");
  }
}
