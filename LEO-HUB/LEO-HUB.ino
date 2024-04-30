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
#include "TcBUZZER.h"

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

// ----------------- VARIABLE ----------------- //
uint8_t passToneCount = 0;
uint8_t ngToneCount = 0;
const uint8_t totalToneNG = 15;
uint32_t previousMillis = 0;
uint32_t timeStart, timeComplete = 0;
boolean stateStart, stateStop = false;
boolean stateLockJig = false;
int countLockJig = 0;
int countUnlockJig = 0;
const int countLockJigMax = 1 * 10; // 3 second
boolean stateCensorOnStation = false;
String name = "SELECT ITEM ";
int stdMin, stdMax = 0;
int countScrew = 0;
int countScrewMax = 0;
bool isAllowMes = false;
bool isStarted = false;

bool isCensorOnStation = false;
bool isEndProcess = false;

uint8_t currentSequenceOfTest = 0;
uint8_t totalSequenceOfTest = 0;

// -------------------- FUNCTION -------------------- //
enum SEQUENCE
{
  TESTING,
  PASS,
  NG,
  STOP,
  RESET,
  READY
};
SEQUENCE sequence = READY; // Default state

String item = "";
String id = "";
bool isMenuSetting, oldIsMenuSetting = false;
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

int rfidShow = 0;
String rfidData = "";

int scannerShow = 0;
String scannerData = "";

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

bool IsStateStarted = false;
void LED_Controls(uint8_t);
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

  delay(1000);
  Serial.print("START");
  // Serial1.println("$PWR:ON#");
  // ----------------- BUTTON ----------------- //
  btnCensorOnSt.OnEventChange(btnCensorOnStOnEventChange);
  startButton.OnEventChange(startOnEventChange);
  startButton.DebounceDelay(10); // Set debounce delay to 10ms
  stopButton.OnEventChange(stopOnEventChange);
  stopButton.DebounceDelay(10); // Set debounce delay to 10ms
  endButton.OnEventChange(endOnEventChange);
  endButton.DebounceDelay(2); // Set debounce delay to 5ms

  // ----------------- RELAY ----------------- //
  LED_Controls(0);
  isStarted = false;
  buzzerPass.setTime(200);
  // buzzerNG.setTime(100);
  passToneCount = 2;
  buzzerPass.total = 2;
  // ----------------- SERIAL ----------------- //
}

void loop()
{

  btnCensorOnSt.update();
  startButton.update();
  stopButton.update();
  endButton.update();
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
    if (stateStart && stateStop && stateCensorOnStation)
    {
      stateStart = false;
      stateStop = false;
      timeComplete = currentMillis - timeStart;
      countScrew++;
      if (timeComplete >= stdMin && timeComplete <= stdMax)
      {
        // Check count screw
        if (countScrew >= countScrewMax)
        {
          if (countScrew != countScrewMax)
          {
            isAllowMes = false;
            sequence = NG;
            ngToneCount = totalToneNG;
            // LED ON ledRed
            LED_Controls(1);
          }
        }
        else
        {
          sequence = TESTING;
          LED_Controls(0);
        }
        // LED on ledGreen
        // passToneCount += 1;
        buzzerPass.setTime(200);
        buzzerPass.total += 1;
      }
      else
      {
        // NG
        sequence = NG;
        // LED ON ledRed
        LED_Controls(1);
      }
      String data = "$SAVE:"; // + String(timeComplete) + "#";
      data += ",item:" + String(item);
      data += ",time_start:" + String(timeStart);
      data += ",time_end:" + String(currentMillis);
      data += ",time_complete:" + String(timeComplete);
      data += ",scw:" + String(countScrew);
      data += ",judgement:" + String((sequence == TESTING || sequence == PASS) ? "PASS" : "NG");
      data += "#";
      Serial.println(data);
      mySerial.println(data);

      if (timeComplete >= stdMin && timeComplete <= stdMax)
      {
        // Check count screw
        if (countScrew >= countScrewMax)
        {
          if (countScrew == countScrewMax)
          {
            currentSequenceOfTest++;
            if (currentSequenceOfTest >= totalSequenceOfTest)
            {
              /** */
              sequence = PASS;
            }
            else
            {
              sequence = TESTING;
              countScrew = 0;
              Serial.println("Next sequence of test");
              mySerial.println("$SEQ:NEXT#");
            }
          }
        }
      }
    }

    if (sequence == NG)
    {
      buzzerPass.total = 15;
    }
  }

  // check ubuntu starting to return
  if (isStarted == false)
  {
    return;
  }
  //
  buzzerPass.update();

  if (isEndProcess && isCensorOnStation)
  {
    /** End process */
    isEndProcess = false;
    isCensorOnStation = false;

    if (sequence == PASS)
    {
      isAllowMes = true;
      // passToneCount = 1;
      buzzerPass.setTime(200);
      buzzerPass.total += 2;
      countUnlockJig = countLockJigMax;
      // LED ON ledGreen
      LED_Controls(2);
      Serial.println("Complete");

      isCensorOnStation = false;

      // After stop, reset all value to default and check screw count
      mySerial.println("$SEQ:RST#");
      currentSequenceOfTest = 0;
      countScrew = 0;
      sequence = READY;
      timeComplete = 0;
    

      if(btnCensorOnSt.getState()){
         btnCensorOnStOnEventChange(true);
      }
    }
    else
    {
      sequence = NG;
      // LED ON ledRed
      LED_Controls(1);
      isCensorOnStation = true;
    }
  }

  // ----------------- MODE TSET ----------------- //
  if (currentMillis - previousMillis >= 100)
  {
    if (!isMenuSetting)
    {
      if (stateCensorOnStation)
      {
        if (countLockJig > 0)
        {
          countLockJig--;
          if (countLockJig <= 1)
          {
            countUnlockJig = 0;
            relayLockJig.on();
            relayTorquePwr.on();
          }
        }
        else if (countUnlockJig > 0)
        {
          countUnlockJig--;
          if (countUnlockJig <= 1)
          {
            relayLockJig.off();
            relayTorquePwr.off();
            countLockJig = 0; //
          }
        }
      }else{
        relayLockJig.off();
        relayTorquePwr.off();
      }
      if (oldIsMenuSetting != isMenuSetting)
      {
        oldIsMenuSetting = isMenuSetting;
        if (sequence == TESTING)
        {
          relayTorquePwr.on();
        }
      }
      // ------------------ Display ------------------- //
      String line1 = name + ":" + String(stdMin) + "-" + String(stdMax);
      String line2 = "";
      if (stateStart && !stateStop && stateCensorOnStation)
      {
        line2 = "SCW:" + String(countScrew) + "/" + String(countScrewMax) + ":" + String(currentMillis - timeStart) + "ms";
      }
      else if (stateCensorOnStation)
      {
        line2 = "SCW:" + String(countScrew) + "/" + String(countScrewMax) + ":" + String(timeComplete) + "ms";
      }
      else
      {
        line2 = "----------";
      }

      if (rfidShow > 0)
      {
        line1 = "RFID :";
        line2 = rfidData;
        rfidShow--;
      }
      else
      {
        // rfidData = "";
      }
      updateLCD(line1.c_str(), line2.c_str());
    }
    else
    {
      relayTorquePwr.off();
    }
    previousMillis = currentMillis;
  }
  else if (currentMillis < previousMillis)
  {
    previousMillis = currentMillis;
  }
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
    if (stateCensorOnStation)
    {
      mySerial.print("$RFID:");
      mySerial.print(hexStr);
      mySerial.println("#");
      rfidData = hexStr;
      tone(BUZZER_PIN, 2000, 50);
    }
    else
    {
      rfidData = "Not accept";
      ngToneCount = totalToneNG;
      buzzerPass.total = 15;
    }
    rfidShow = 10;
    startReceivedByte1 = false;
    endReceivedByte1 = false;
    // Clear the inputString
    memset(inputByte1, 0, BUFFER_SIZE_DATA);
    inputByteLength1 = 0;
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
    if (!isAllowMes)
    {
      scannerShow = 10;
      scannerData = "Not allow";
      // ngToneCount = 10;
      buzzerPass.total = 10;
      return;
    }
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
    // passToneCount += 1;
    buzzerPass.setTime(200);
    buzzerPass.total += 1;
    scannerShow = 10;
    scannerData = "OK";
  }
  else if (dataInput.indexOf("LCD:") != -1)
  {
    String serialDataLine1 = extractData(dataInput, "line1:");
    String serialDataLine2 = extractData(dataInput, "line2:");
    updateLCD(serialDataLine1, serialDataLine2);
  }
  else if (dataInput.indexOf("P1:") != -1)
  {
    String serialData = extractData(dataInput, "P1:");
    if (serialData == "0")
    {
      isEndProcess = true;
    }
  }
  else if (dataInput.indexOf("RFID:") != -1)
  {
    String serialData = extractData(dataInput, "RFID:");
    if (serialData == "1")
    {
      rfidShow = 10;
      rfidData = "OK";
      isAllowMes = true;
      sequence = RESET;
      countUnlockJig = countLockJigMax;
      buzzerPass.off();

      buzzerPass.setTime(200);
      buzzerPass.total += 1;
      String data = "$LOG:";
      data += ",item:" + item;
      data += ",data: UNLOCK BY RFID " + rfidData;
      data += "#";
      // Serial.println(data);
      mySerial.println(data);

      isCensorOnStation = false;

      // clear

      mySerial.println("$SEQ:RST#");
      currentSequenceOfTest = 0;
      countScrew = 0;
      sequence = READY;
      timeComplete = 0;

    }
    else
    {
      rfidShow = 10;
      rfidData = "Not accept";
    }
  }
  else if (dataInput.indexOf("SETMODE:") != -1)
  {
    String _extractData = extractData(dataInput, "SETMODE:");
    if (_extractData == "1")
    {
      isMenuSetting = true;
      oldIsMenuSetting = true;
    }
    else
    {
      isMenuSetting = false;
    }
  }
  else if (dataInput.indexOf("TONE:") != -1)
  {
    String _extractData = extractData(dataInput, "TONE:");
    if (_extractData == "BTN")
    {
      tone(BUZZER_PIN, 2000, 100);
    }
  }
  else if (dataInput.indexOf("ITEM:") != -1)
  {
    String _extractData = extractData(dataInput, "ITEM:");
    if (_extractData.length() > 0)
    {
      item = _extractData;
    }
  }
  else if (dataInput.indexOf("CONN:") != -1)
  {
    mySerial.println("$CONN:OK#");
  }
  else if (dataInput.indexOf("PWR:") != -1)
  {
    // String _extractData = extractData(dataInput, "PWR:");
    if (isStarted == false)
    {
      mySerial.println("$SEQ:RST#"); // Reset sequence
    }
    isStarted = true;
  }

  else if (dataInput.indexOf("UPDATE:") != -1)
  {
    String _extractData = extractData(dataInput, "MIN:");
    if (_extractData.length() > 0)
    {
      stdMin = _extractData.toInt();
    }

    _extractData = extractData(dataInput, "MAX:");
    if (_extractData.length() > 0)
    {
      stdMax = _extractData.toInt();
    }

    _extractData = extractData(dataInput, "SCW:");
    if (_extractData.length() > 0)
    {
      countScrewMax = _extractData.toInt();
    }
    _extractData = extractData(dataInput, "SEQ:");
    if (_extractData.length() > 0)
    {
      totalSequenceOfTest = _extractData.toInt();
    }
    _extractData = extractData(dataInput, "NAME:");
    if (_extractData.length() > 0)
    {
      name = _extractData;
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

void LED_Controls(uint8_t state = 0)
{

  // Serial.println("LED_Controls: " + String(state == 0 ? "OFF" : state == 1 ? "RED" : state == 2 ? "GREEN" : "BLUE"));
  if (state == 0)
  {
    ledRed.off();
    ledGreen.off();
    ledBlue.off();

    relayGreen.off();
    relayBlue.on();
    relayRed.off();
    relayAram.off();
  }
  else if (state == 1)
  {
    ledRed.on();
    ledGreen.off();
    ledBlue.off();

    relayGreen.off();
    relayBlue.off();
    relayRed.on();
    relayAram.on();
  }
  else if (state == 2)
  {
    ledRed.off();
    ledGreen.on();
    ledBlue.off();

    relayGreen.on();
    relayBlue.off();
    relayRed.off();
  }
  else if (state == 3)
  {
    ledRed.off();
    ledGreen.off();
    ledBlue.on();
  }
}

void btnCensorOnStOnEventChange(bool state)
{
  stateCensorOnStation = !state;
  if(!isCensorOnStation){
    isCensorOnStation = stateCensorOnStation;
  }

  if (!stateCensorOnStation)
  {
    Serial.println("--------LED OFF STD ----------");
    relayLockJig.off();
    relayTorquePwr.off();
    String data = "$LOG:";
    data += ",item:" + item;
    data += ",data: SENSOR OFF ";
    data += "#";
    mySerial.println(data);
    isAllowMes = false;

    LED_Controls(0);
    buzzerPass.off();
  }
  else
  {
    Serial.println("--------LED ON STD ----------");
    String data = "$LOG:";
    data += ",item:" + item;
    data += ",data: SENSOR ON ";
    data += "#";
    if(sequence != NG ){
    countLockJig = countLockJigMax;
    }
    mySerial.println(data);
    isAllowMes = false;
    if(!isCensorOnStation){
      previousMillis = millis();
      sequence = TESTING;
      mySerial.println("$ITEM:GET#");
    }
  }
  isEndProcess = false;
}

void startOnEventChange(bool state)
{
  stateStart = !state;
  if (stateStart)
  {
    timeStart = millis(); // Stamp time start
    LED_Controls(3);
    String data = "$LOG:";
    data += ",item:" + item;
    data += ",data: START " + String(millis());
    data += "#";
    // Serial.println(data);
    mySerial.println(data);
  }
  else if (sequence == TESTING)
  {
    // Serial.println("--------LED OFF STD ----------");
    LED_Controls(0);
  }
  else if (sequence == PASS || sequence == NG || countScrew >= countScrewMax)
  {
    relayTorquePwr.off();
  }
}

void stopOnEventChange(bool state)
{
  stateStop = !state;
  if (stateStop)
  {
    String data = "$LOG:";
    data += ",item:" + item;
    data += ",data: STOP " + String(millis());
    data += "#";
    // Serial.println(data);
    mySerial.println(data);
  }
}


void endOnEventChange(bool state)
{
  if (state)
  {
    parseData("P1:0");
    Serial.println("end true");
  }
}