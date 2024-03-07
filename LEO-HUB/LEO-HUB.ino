/**
 * @file LEO-HUB.ino
 * @author Jakkapan
 * @brief
 * @version 0.1
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
#include "Keyboard.h"
#include <SPI.h>
#include <SoftwareSerial.h>


#define SENSOR_PIN 7
#define START_PIN 8
#define STOP_PIN 9
#define RELAY_RED_PIN A0
#define RELAY_GREEN_PIN A1
#define RELAY_BLUE_PIN A2
#define RELAY_ALARM_PIN A3
#define RELAY_LOCK_JIG_PIN A4
#define RELAY_TORQUE_PWR_PIN A5

#define BUZZER_PIN 6
uint8_t passToneCount = 0;
uint32_t lastTimeTonePASS = 0;
uint8_t ngToneCount = 0;
uint32_t lastTimeToneNG = 0;

enum Sequence
{
  TESTING,
  PASS,
  NG,
  STOP,
  RESET,
  READY
};



bool isMenuSetting = false;
#define TX_PIN 10
#define RX_PIN 11
SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX
// -------------------- FUNCTION -------------------- //
#define BUFFER_SIZE_DATA 60
// -------------------- LCD -------------------- //
LiquidCrystal_I2C lcd(0x27, 16, 2);
// -------------------- SERIAL  -------------------- //
boolean startReceived = false;
boolean endReceived = false;

const char startChar = '$';
const char endChar = '#';

char inputString[BUFFER_SIZE_DATA];
int inputStringLength = 0;
void serialEvent()
{
  while (Serial.available())
  {
    byte inChar = (byte)Serial.read();
    if (inChar == startChar)
    {
      startReceived = true;
      inputStringLength = 0;
    }
    else if (startReceived && inChar == endChar)
    {
      endReceived = true;
    }
    else if (startReceived)
    {
      if (inputStringLength < BUFFER_SIZE_DATA - 1)
      {
        inputString[inputStringLength++] = inChar;
      }
      else
      {
        // startReceived = false;
        endReceived = false;
        inputStringLength = 0;
      }
    }
  }
}

// -------------------- SERIAL 1 -------------------- //
bool startReceived1 = false;
bool endReceived1 = false;
const char startChar1 = '$';
const char endChar1 = '#';
// String inputString1 = "";
char inputString1[BUFFER_SIZE_DATA];
int inputStringLength1 = 0;

bool startReceivedByte1 = false;
bool endReceivedByte1 = false;
const byte startByte1 = 0x02;
const byte endByte1 = 0x03;
byte inputByte1[BUFFER_SIZE_DATA];
int inputByteLength1 = 0;
void serialEvent1()
{
  while (Serial1.available())
  {
    byte inChar = (byte)Serial1.read();
    if (inChar == startChar1)
    {
      startReceived1 = true;
      inputStringLength1 = 0;
    }
    else if (startReceived1 && inChar == endChar1)
    {
      endReceived1 = true;
    }
    else if (startReceived1)
    {
      if (inputStringLength1 < BUFFER_SIZE_DATA - 1)
      {
        inputString1[inputStringLength1++] = (char)inChar;
      }
      else
      {
        // startReceived1 = false;
        endReceived1 = false;
        inputStringLength1 = 0;
      }
    }
    // ----------------- BYTE ----------------- //
    if (inChar == startByte1)
    {
      startReceivedByte1 = true;
      inputByteLength1 = 0;
    }
    else if (startReceivedByte1 && inChar == endByte1)
    {
      endReceivedByte1 = true;
    }
    else if (startReceivedByte1)
    {
      if (inputByteLength1 < BUFFER_SIZE_DATA - 1)
      {
        inputByte1[inputByteLength1++] = inChar;
      }
      else
      {
        startReceivedByte1 = false;
        endReceivedByte1 = false;
        inputByteLength1 = 0;
      }
    }
  }
  // Serial.println("");
}

bool startReceivedI2c = false;
bool endReceivedI2c = false;
const char startCharI2c = '$';
const char endCharI2c = '#';
char inputStringI2c[BUFFER_SIZE_DATA];
int inputStringLengthI2c = 0;
// --------------------- Serial SW -----------------//
bool startReceivedSW = false;
bool endReceivedSW = false;
const char startCharSW = '$';
const char endCharSW = '#';
char inputStringSW[BUFFER_SIZE_DATA];
int inputStringLengthSW = 0;
void serialEventSW()
{
  while (mySerial.available())
  {
    byte inChar = (byte)mySerial.read();
    if (inChar == startCharSW)
    {
      startReceivedSW = true;
      inputStringLengthSW = 0;
    }
    else if (startReceivedSW && inChar == endCharSW)
    {
      endReceivedSW = true;
    }
    else if (startReceivedSW)
    {
      if (inputStringLengthSW < BUFFER_SIZE_DATA - 1)
      {
        inputStringSW[inputStringLengthSW++] = (char)inChar;
      }
      else
      {
        // startReceivedSW = false;
        endReceivedSW = false;
        inputStringLengthSW = 0;
      }
    }
  }
}



void setup()
{
  lcd.begin();
  lcd.clear();
  lcd.backlight();
  Serial.begin(9600);
  Serial1.begin(9600);
  mySerial.begin(9600);

  memset(inputString, 0, BUFFER_SIZE_DATA);
  memset(inputString1, 0, BUFFER_SIZE_DATA);
  memset(inputStringI2c, 0, BUFFER_SIZE_DATA);
  memset(inputByte1, 0, BUFFER_SIZE_DATA);
  memset(inputStringSW, 0, BUFFER_SIZE_DATA);

  Keyboard.begin();
  updateLCD("Starting...", "Please wait....");

  passToneCount = 2;
  delay(1000);
  Serial.print("START");
  Serial1.println("$PWR:ON#");
}

uint32_t previousMillis = 0;

uint32_t timeStart, timeComplete = 0;
boolean stateStart, stateStop = false;
boolean stateLockJig = false;
int countLockJig = 0;
int countUnlockJig = 0;
const int countLockJigMax = 3;
boolean stateCensorOnStation = false;
String name ="T002";
int stdMin, stdMax = 0 ;
int scw_c = 0;
int scw_count = 0;
void loop()
{
  // -- Serial SW -- //
  serialEventSW();

  manageSerial();
  manageSerial1();
  manageSerialI2c();
  manageByteSerial1();
  manageSWSerial();

  uint32_t currentMillis = millis();

  if (!isMenuSetting)
  {

  }
  // ----------------- MODE TSET ----------------- //
  if (currentMillis - previousMillis >= 100)
  {
    if (!isMenuSetting)
    {
      String line1 = name+" STD:" + String(stdMin) + " - " + String(stdMax);
      String line2 = "";
      if (stateStart && !stateStop && stateCensorOnStation)
      {
        line2 = "SCW: " + String(scw_c) + "/" + String(scw_count)+":"+ String(currentMillis - timeStart)+"ms";
      }else if(stateCensorOnStation){
        line2 = "SCW: " + String(scw_c) + "/" + String(scw_count) + ":"+String(timeComplete)+"ms";
      }else{
        line2 = "----------";
      }
      updateLCD(line1.c_str(), line2.c_str());
    }else{

    }
    // mySerial.print("$->Hello#");
    previousMillis = currentMillis;
  }
  else if (currentMillis < previousMillis)
  {
    previousMillis = currentMillis;
  }

  ToneFun(currentMillis, lastTimeTonePASS, 200, 2000, 50, passToneCount); //
  ToneFun(currentMillis, lastTimeToneNG, 100, 2000, 50, ngToneCount);
}

void manageSerial()
{
  if (startReceived && endReceived)
  {
    inputString[inputStringLength] = '\0';
    Serial.print("Send to Serial1: ");
    Serial.println(inputString);
    Serial1.println(inputString);
    startReceived = false;
    endReceived = false;
    // Clear the inputString
    memset(inputString, 0, BUFFER_SIZE_DATA);
    inputStringLength = 0;
  }
}

void manageByteSerial1()
{
  if (startReceivedByte1 && endReceivedByte1)
  {
    inputByte1[inputByteLength1] = '\0';
    char hexStr[(inputByteLength1 * 2) + 1];

    memset(hexStr, 0, sizeof(hexStr)); // Clear hexStr
    for (int i = 0; i < inputByteLength1; i++)
    {
      // Serial.print(inputByte1[i], HEX);
      // Serial.print(" ");
      sprintf(&hexStr[i * 2], "%02X", inputByte1[i]);
    }
    // Serial.println(hexStr);
    Serial.print("$RFID:");
    Serial.print(hexStr);
    Serial.println("#");
    mySerial.print("$RFID:");
    mySerial.print(hexStr);
    mySerial.println("#");
    startReceivedByte1 = false;
    endReceivedByte1 = false;
    // Clear the inputString
    memset(inputByte1, 0, BUFFER_SIZE_DATA);
    inputByteLength1 = 0;
    tone(BUZZER_PIN, 2000, 50);
  }
}

void ToneFun(uint32_t _currentMillis, uint32_t &_lastTime, uint32_t _toneTime, int _toneFreq, uint8_t _dutyCycle, uint8_t &totalTone)
{
  if (totalTone <= 0)
  {
    return;
  }
  // uint32_t currentMillis = millis();
  if (_currentMillis - _lastTime > _toneTime)
  {
    if (_dutyCycle > 0)
    {
      int p = (int)(_dutyCycle * _toneTime / 100);
      tone(BUZZER_PIN, _toneFreq, p);
      totalTone--;
    }
    _lastTime = _currentMillis;
  }
  else if (_currentMillis < _lastTime)
  {
    _lastTime = _currentMillis;
  }
}

void manageSerial1()
{
  if (startReceived1 && endReceived1)
  {
    inputString1[inputStringLength1] = '\0';
    Serial.print("Received from Serial1: ");
    Serial.println(inputString1);
    parseData(inputString1);
    startReceived1 = false;
    endReceived1 = false;
    // Clear the inputString
    memset(inputString1, 0, BUFFER_SIZE_DATA);
    inputStringLength1 = 0;
  }
}

void manageSerialI2c()
{
  if (startReceivedI2c && endReceivedI2c)
  {
    inputStringI2c[inputStringLengthI2c] = '\0';
    Serial.print("Received from I2C: ");
    Serial.println(inputStringI2c);

    parseData(inputStringI2c);
    startReceivedI2c = false;
    endReceivedI2c = false;
    // Clear the inputString
    memset(inputStringI2c, 0, BUFFER_SIZE_DATA);
    inputStringLengthI2c = 0;
  }
}

void manageSWSerial()
{
  if (startReceivedSW && endReceivedSW)
  {
    inputStringSW[inputStringLengthSW] = '\0';
    Serial.print("Received from SW Serial: ");
    Serial.println(inputStringSW);
    parseData(inputStringSW);
    startReceivedSW = false;
    endReceivedSW = false;
    // Clear the inputString
    memset(inputStringSW, 0, BUFFER_SIZE_DATA);
    inputStringLengthSW = 0;
  }
}

void parseData(String dataInput)
{
  dataInput.trim();
  if (dataInput.indexOf("KBD_K:") != -1)
  {
    String serialData = extractData(dataInput, "KBD_K:");
    Serial.print("DataEx: ");
    Serial.println(serialData);
    if (serialData.length() > 0)
    {
      for (int i = 0; i < serialData.length(); i++)
      {
        Keyboard.print(serialData[i]);
      }
      Keyboard.press(KEY_RETURN);
      Keyboard.releaseAll();
    }
  }
  else if (dataInput.indexOf("LCD:") != -1)
  {
    String serialDataLine1 = extractData(dataInput, "line1:");
    String serialDataLine2 = extractData(dataInput, "line2:");
    updateLCD(serialDataLine1, serialDataLine2);
  }

  else if (dataInput.indexOf("SETMODE:") != -1)
  {
    String _extractData = extractData(dataInput, "SETMODE:");
    if(_extractData == "1"){
      isMenuSetting = true;
      }else{
      isMenuSetting = false;
      }
  }else if (dataInput.indexOf("TONE:") != -1)
  {
    String _extractData = extractData(dataInput, "TONE:");
    if(_extractData == "BTN"){
       tone(BUZZER_PIN, 2000, 100);
      }
  }

}

String extractData(String dataInput, String key)
{
  int keyIndex = dataInput.indexOf(key); // Find the position of the key
  if (keyIndex == -1)
  {
    return ""; // Return 0 if key not found
  }

  int startIndex = keyIndex + key.length();          // Start index for the number
  int endIndex = dataInput.indexOf(",", startIndex); // Find the next comma after the key
  if (endIndex == -1)
  {
    endIndex = dataInput.length(); // If no comma, assume end of string
  }

  String valueStr = dataInput.substring(startIndex, endIndex); // Extract the substring
  return valueStr;                                             // Return the extracted string
}

char currentLine1[17] = "                "; // 16 characters + null terminator
char currentLine2[17] = "                "; // 16 characters + null terminator

void updateLCD(const String newDataLine1, const String newDataLine2)
{
  updateLCD(newDataLine1.c_str(), newDataLine2.c_str());
}

void updateLCD(const char *newDataLine1, const char *newDataLine2)
{
  updateLCDLine(newDataLine1, currentLine1, 0);
  updateLCDLine(newDataLine2, currentLine2, 1);
}

void updateLCDLine(const char *newData, char (&currentLine)[17], int row)
{
  int i;
  // Update characters as long as they are different or until newData ends
  for (i = 0; i < 16 && newData[i]; i++)
  {
    if (newData[i] != currentLine[i])
    {
      lcd.setCursor(i, row);
      lcd.print(newData[i]);
      currentLine[i] = newData[i];
    }
  }
  // Clear any remaining characters from the previous display
  for (; i < 16; i++)
  {
    if (currentLine[i] != ' ')
    {
      lcd.setCursor(i, row);
      lcd.print(' ');
      currentLine[i] = ' ';
    }
  }
}
