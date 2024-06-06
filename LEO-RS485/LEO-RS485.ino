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
TcBUTTON btnCensorOnSt(BTN_CENSOR_ON_ST, true);

#define START_PIN 8
void startOnEventChange(bool state);
TcBUTTON startButton(START_PIN, false);

#define STOP_PIN 9
void stopOnEventChange(bool state);
TcBUTTON stopButton(STOP_PIN, false);

#define END_PIN 12
void endOnEventChange(bool state);
TcBUTTON endButton(END_PIN, true);

// -------------------- RELAY -------------------- //
#define LED_RED_PIN 4
TcPINOUT ledRed(LED_RED_PIN, false);

#define LED_GREEN_PIN 5
TcPINOUT ledGreen(LED_GREEN_PIN, false);

#define LED_BLUE_PIN 13
TcPINOUT ledBlue(LED_BLUE_PIN, false);

#define RELAY_RED_PIN A0
TcPINOUT relayRed(RELAY_RED_PIN, false);

#define RELAY_GREEN_PIN A1
TcPINOUT relayGreen(RELAY_GREEN_PIN, false);

#define RELAY_BLUE_PIN A2
TcPINOUT relayBlue(RELAY_BLUE_PIN, false);

#define RELAY_ALARM_PIN A3
TcPINOUT relayAram(RELAY_ALARM_PIN, false);

#define RELAY_LOCK_JIG_PIN A4
TcPINOUT relayLockJig(RELAY_LOCK_JIG_PIN, false);

#define RELAY_TORQUE_PWR_PIN A5
TcPINOUT relayTorquePwr(RELAY_TORQUE_PWR_PIN, false);

#define BUZZER_PIN 6
TcBUZZER buzzerPass(BUZZER_PIN, false);

#define RX_PIN 10
#define TX_PIN 11
SoftwareSerial mySerial(RX_PIN, TX_PIN);  // RX, TX

const uint8_t slaveID = 1;

ModbusRTUSlave modbus(mySerial);
bool coils[10] = {false};
bool discreteInputs[10] = {false};
uint16_t holdingRegisters[10] = {0};

void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);
  mySerial.begin(9600);

  modbus.begin(slaveID, 9600);
  modbus.configureCoils(coils, 10);
  modbus.configureDiscreteInputs(discreteInputs, 10);
  modbus.configureHoldingRegisters(holdingRegisters, 10);

  btnCensorOnSt.OnEventChange(btnCensorOnStOnEventChange);
  startButton.OnEventChange(startOnEventChange);
  startButton.DebounceDelay(10);  // Set debounce delay to 10ms
  stopButton.OnEventChange(stopOnEventChange);
  stopButton.DebounceDelay(10);  // Set debounce delay to 10ms
  endButton.OnEventChange(endOnEventChange);
  endButton.DebounceDelay(2);  // Set debounce delay to 5ms
}
int count = 0;
uint8_t oldStateType = 0;
void loop() {
  btnCensorOnSt.update();
  startButton.update();
  stopButton.update();
  endButton.update();
  modbus.poll();
  count++;
  holdingRegisters[0] = count;
  if (count >= 1000) {
    count = 0;
  }
  ReadRFID();
 if(coils[1] == false && discreteInputs[1] == true)
 {
    // Clear card data
    holdingRegisters[1] = 0;
    holdingRegisters[2] = 0;
    holdingRegisters[3] = 0;

    Serial.println("Card data cleared");
    
    coils[1] = false;
    discreteInputs[1] = false;
 }
}

void ReadRFID() {
   // Non-blocking RFID data reading
  if (Serial1.available() >= 9) { // Ensure there are at least 9 bytes to read
    if (Serial1.read() == 0x02) { // Start of data
      byte length = Serial1.read();
      if (length == 0x09) {
        byte cardType = Serial1.read();
        byte cardData[4];
        for (int i = 0; i < 4; i++) {
          cardData[i] = Serial1.read();
        }
        byte bcc = Serial1.read();
        byte end = Serial1.read();

        if (end == 0x03) { // End of data
          // Verify BCC
          byte bccCheck = length ^ cardType ^ cardData[0] ^ cardData[1] ^ cardData[2] ^ cardData[3];
          if (bcc == bccCheck) {
     
            
            Serial.print("Card Type: ");
            Serial.println(cardType == 0x02 ? "EM4100" : "Mifare 1K");
            
            // Convert card data to a single integer
            uint32_t cardNumber = (uint32_t)cardData[0] << 24 | (uint32_t)cardData[1] << 16 | (uint32_t)cardData[2] << 8 | cardData[3];
            
            Serial.print("Card Data: ");
            Serial.println(cardNumber); // Print card number in decimal

             // Store the card number into holding registers
            holdingRegisters[1] = (uint16_t)(cardNumber >> 16); // Upper 16 bits
            holdingRegisters[2] = (uint16_t)(cardNumber & 0xFFFF); // Lower 16 bits
            holdingRegisters[3] = cardType; // Card type (0x02 for EM4100, 0x03 for Mifare 1K) 
            // holdingRegisters[4] = 1; // Card data ready flag

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
  coils[7] = true;
  //   stateCensorOnStation = !state;
  //   if(!isCensorOnStation){
  //     isCensorOnStation = stateCensorOnStation;
  //   }

  //   if (!stateCensorOnStation)
  //   {
  //     Serial.println("--------LED OFF STD ----------");
  //     relayLockJig.off();
  //     relayTorquePwr.off();
  //     String data = "$LOG:";
  //     data += ",item:" + item;
  //     data += ",data: SENSOR OFF ";
  //     data += "#";
  //     mySerial.println(data);
  //     isAllowMes = false;

  //     LED_Controls(0);
  //     buzzerPass.off();
  //   }
  //   else
  //   {
  //     Serial.println("--------LED ON STD ----------");
  //     String data = "$LOG:";
  //     data += ",item:" + item;
  //     data += ",data: SENSOR ON ";
  //     data += "#";
  //     if(sequence != NG ){
  //     countLockJig = countLockJigMax;
  //     }
  //     mySerial.println(data);
  //     isAllowMes = false;
  //     // if(!isCensorOnStation){
  //     //   previousMillis = millis();
  //     //   sequence = TESTING;
  //     //   mySerial.println("$ITEM:GET#");
  //     // }
  //   }
  //   isEndProcess = false;
}

void startOnEventChange(bool state) {
  discreteInputs[5] = !state;
  coils[5] = true;
  //   stateStart = !state;
  //   if (stateStart)
  //   {
  //     timeStart = millis(); // Stamp time start
  //     LED_Controls(3);
  //     String data = "$LOG:";
  //     data += ",item:" + item;
  //     data += ",data: START " + String(millis());
  //     data += "#";
  //     // Serial.println(data);
  //     mySerial.println(data);
  //   }
  //   else if (sequence == TESTING)
  //   {
  //     // Serial.println("--------LED OFF STD ----------");
  //     LED_Controls(0);
  //   }
  //   else if (sequence == PASS || sequence == NG || countScrew >= countScrewMax)
  //   {
  //     relayTorquePwr.off();
  //   }
}

void stopOnEventChange(bool state) {
  discreteInputs[6] = !state;
  coils[6] = true;
  //   stateStop = !state;
  //   if (!state)
  //   {
  //     stateStop = true;
  //     String data = "$LOG:";
  //     data += ",item:" + item;
  //     data += ",data: STOP " + String(millis());
  //     data += "#";
  //     // Serial.println(data);
  //     mySerial.println(data);
  //   }else{
  //     stateStop =false;
  //   }
}


void endOnEventChange(bool state) {
}