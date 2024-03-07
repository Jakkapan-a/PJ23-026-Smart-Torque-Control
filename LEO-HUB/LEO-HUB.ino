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
// #include <LiquidCrystal_I2C.h>
#include "Keyboard.h"

// // -------------------- INPUT -------------------- //
// #define BTN_START 11
// void btnStartOnEventChange(bool state);
// TcBUTTON btnStart(BTN_START, false);

// #define BTN_STOP 12
// void btnStopOnEventChange(bool state);
// TcBUTTON btnStop(BTN_STOP, false);

// #define BTN_ESC_PIN A0
// void btnEscOnEventChange(bool state);
// TcBUTTON btnEsc(BTN_ESC_PIN, false);

// #define BTN_UP_PIN A1
// void btnUpOnEventChange(bool state);
// TcBUTTON btnUp(BTN_UP_PIN, false);

// #define BTN_DOWN_PIN A2
// void btnDownOnEventChange(bool state);
// TcBUTTON btnDown(BTN_DOWN_PIN, false);

// #define BTN_ENTER_PIN A3
// void btnEnterOnEventChange(bool state);
// TcBUTTON btnEnter(BTN_ENTER_PIN, false);

// // ---------------------- OUTPUT ---------------------- //
// #define IO_OUT_MES_PIN 4
// void ledMesOnEventChange(bool state);
// TcPINOUT ledMes(IO_OUT_MES_PIN, false);

// #define IO_OUT_PWR_PIN 5 // Power torque
// void ledPwrOnEventChange(bool state);
// TcPINOUT ledPwr(IO_OUT_PWR_PIN, false);

// #define IO_OUT_RED_PIN 7
// void ledRedOnEventChange(bool state);
// TcPINOUT ledRed(IO_OUT_RED_PIN, false);

// #define IO_OUT_GREEN_PIN 8
// void ledGreenOnEventChange(bool state);
// TcPINOUT ledGreen(IO_OUT_GREEN_PIN, false);

// #define IO_OUT_BLUE_PIN 9 // Blue or Yellow
// void ledBlueOnEventChange(bool state);
// TcPINOUT ledBlue(IO_OUT_BLUE_PIN, false);

// #define IO_OUT_LOG_JIG_PIN 10
// void ledLogJigOnEventChange(bool state);
// TcPINOUT ledLogJig(IO_OUT_LOG_JIG_PIN, false);

#define BUZZER_PIN 6
uint8_t passToneCount = 0;
uint32_t lastTimeTonePASS = 0;
uint8_t ngToneCount = 0;
uint32_t lastTimeToneNG = 0;

bool isMenuSetting = false;

// -------------------- FUNCTION -------------------- //
#define BUFFER_SIZE_DATA 100
// -------------------- LCD -------------------- //
// LiquidCrystal_I2C lcd(0x27, 16, 2);
// -------------------- SERIAL  -------------------- //
boolean startReceived = false;
boolean endReceived = false;

const char startChar = '$';
const char endChar = '#';

int stdMin, stdMax, scw_count,scw_c = 0;
String name = "";
// String inputString = "";
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

enum STATUS_TEST
{
  PASS,
  NG,
  TESTING,
  NO_TEST,
  STOP
};
STATUS_TEST status_test = NO_TEST;

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
    // Serial.print(inChar, DEC);
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

// -------------------- I2C -------------------- //
void receiveEvent(int howMany)
{
  // while (1 < Wire.available()) { // Loop through all but the last
  //   char c = Wire.read(); // Receive byte as a character
  //   Serial.print(c);      // Print the character to the serial monitor
  // }
  // char x = Wire.read();    // Receive byte as an integer
  // Serial.println(x);      // Print the integer to the serial monitor
  while (Wire.available())
  { // loop through all but the last
    char inChar = (char)Wire.read();
    if (inChar == startCharI2c)
    {
      startReceivedI2c = true;
      inputStringLengthI2c = 0;
    }
    else if (startReceivedI2c && inChar == endCharI2c)
    {
      endReceivedI2c = true;
    }
    else if (startReceivedI2c)
    {
      if (inputStringLengthI2c < BUFFER_SIZE_DATA - 1)
      {
        inputStringI2c[inputStringLengthI2c++] = inChar;
      }
      else
      {
        // startReceivedI2c = false;
        endReceivedI2c = false;
        inputStringLengthI2c = 0;
      }
    }
  }
}
bool isModeDisplay = false;

void requestEvent()
{
  Wire.write("hello "); // Respond with a message of 6 characters
}

void setup()
{

  // lcd.begin();
  // lcd.clear();
  // lcd.backlight();
  Wire.begin(0x74);             // Set the Arduino I2C address to 0x04
  Wire.onReceive(receiveEvent); // Register a receive event handler
  Wire.onRequest(requestEvent); // Register a request event handler
  // Initialize serial communication at 9600 baud rate
  Serial.begin(9600);
  Serial1.begin(9600);

  memset(inputString, 0, BUFFER_SIZE_DATA);
  memset(inputString1, 0, BUFFER_SIZE_DATA);
  memset(inputStringI2c, 0, BUFFER_SIZE_DATA);
  memset(inputByte1, 0, BUFFER_SIZE_DATA);

  Keyboard.begin();
  //updateLCD("Starting...", "Please wait....");
  // btnStart.OnEventChange(btnStartOnEventChange);
  // btnStart.DebounceDelay(10); // 10ms

  // btnStop.OnEventChange(btnStopOnEventChange);
  // btnStop.DebounceDelay(10); // 10ms

  // btnEsc.OnEventChange(btnEscOnEventChange);
  // btnUp.OnEventChange(btnUpOnEventChange);
  // btnDown.OnEventChange(btnDownOnEventChange);
  // btnEnter.OnEventChange(btnEnterOnEventChange);

  // ledLogJig.setCallback(ledLogJigOnEventChange);
  // ledMes.setCallback(ledMesOnEventChange);
  // ledPwr.setCallback(ledPwrOnEventChange);
  // ledRed.setCallback(ledRedOnEventChange);
  // ledGreen.setCallback(ledGreenOnEventChange);
  // ledBlue.setCallback(ledBlueOnEventChange);

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
void loop()
{
  manageSerial();
  manageSerial1();
  manageSerialI2c();
  manageByteSerial1();
  // btnStart.update();
  // btnStop.update();
  // btnEsc.update();
  // btnUp.update();
  // btnDown.update();
  // btnEnter.update();

  uint32_t currentMillis = millis();
  // ----------------- MODE TSET ----------------- //

  if (currentMillis - previousMillis >= 100)
  {
    // if (!isMenuSetting)
    // {
    //   String line1 = name+" STD: " + String(stdMin) + " - " + String(stdMax);
    //   String line2 = "";
    //   if (stateStart && !stateStop && stateCensorOnStation)
    //   {
    //     line2 = "SCW: " + String(scw_c) + "/" + String(scw_count)+":"+ String(currentMillis - timeStart)+"ms";
    //   }else if(stateCensorOnStation){
    //     line2 = "SCW: " + String(scw_c) + "/" + String(scw_count) + ":"+String(timeComplete)+"ms";
    //   }else{
    //     line2 = "----------";
    //   }
    //   updateLCD(line1.c_str(), line2.c_str());
    // }else{

    // }
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

    Serial1.print("$RFID:");
    Serial1.print(hexStr);
    Serial1.println("#");

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
    // String serialDataLine1 = extractData(dataInput, "line1:");
    // String serialDataLine2 = extractData(dataInput, "line2:");
    // updateLCD(serialDataLine1, serialDataLine2);
  }
  else if (dataInput.indexOf("SETMODE:") != -1)
  {
    String serialData = extractData(dataInput, "SETMODE:");
    if (serialData == "SETTING")
    {
      isMenuSetting = true;
    }
    else
    {
      isMenuSetting = false;
    }
  }
  else if (dataInput.indexOf("UPDATE:") != -1)
  {
    if (dataInput.indexOf("name:") != -1)
    {
      String serialData = extractData(dataInput, "name:");
      name = serialData;
    }

    if (dataInput.indexOf("min:") != -1)
    {
      String serialData = extractData(dataInput, "min:");
      stdMin = serialData.toInt();
    }

    if (dataInput.indexOf("max:") != -1)
    {
      String serialData = extractData(dataInput, "max:");
      stdMax = serialData.toInt();
    }

    if (dataInput.indexOf("scw_c:") != -1)
    {
      String serialData = extractData(dataInput, "scw_c:");
      scw_count = serialData.toInt();
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

// char currentLine1[17] = "                "; // 16 characters + null terminator
// char currentLine2[17] = "                "; // 16 characters + null terminator

// void updateLCD(const String newDataLine1, const String newDataLine2)
// {
//   updateLCD(newDataLine1.c_str(), newDataLine2.c_str());
// }

// void updateLCD(const char *newDataLine1, const char *newDataLine2)
// {
//   updateLCDLine(newDataLine1, currentLine1, 0);
//   updateLCDLine(newDataLine2, currentLine2, 1);
// }

// void updateLCDLine(const char *newData, char (&currentLine)[17], int row)
// {
//   int i;
//   // Update characters as long as they are different or until newData ends
//   for (i = 0; i < 16 && newData[i]; i++)
//   {
//     if (newData[i] != currentLine[i])
//     {
//       lcd.setCursor(i, row);
//       lcd.print(newData[i]);
//       currentLine[i] = newData[i];
//     }
//   }
//   // Clear any remaining characters from the previous display
//   for (; i < 16; i++)
//   {
//     if (currentLine[i] != ' ')
//     {
//       lcd.setCursor(i, row);
//       lcd.print(' ');
//       currentLine[i] = ' ';
//     }
//   }
// }

void btnStartOnEventChange(bool state)
{stateStart = !state;
  if (stateStart)
  {
    timeStart = millis(); // Stamp time start
    LED_Controls(3);
    Serial.println("START");
    Serial1.println("$BTN:START#");
    tone(BUZZER_PIN, 2000, 50);
  }
}

void btnStopOnEventChange(bool state)
{
  if (!state)
  {
    Serial.println("STOP");
    Serial1.println("$BTN:STOP#");
    tone(BUZZER_PIN, 2000, 50);
  }
}

void btnEscOnEventChange(bool state)
{
  if (!state)
  {
    Serial.println("ESC");
    Serial1.println("$BTN:ESC#");
    tone(BUZZER_PIN, 2000, 100);
  }
}

void btnUpOnEventChange(bool state)
{
  if (!state)
  {
    Serial.println("UP");
    Serial1.println("$BTN:UP#");
    tone(BUZZER_PIN, 2000, 100);
  }
}

void btnDownOnEventChange(bool state)
{
  if (!state)
  {
    Serial.println("DOWN");
    Serial1.println("$BTN:DOWN#");
    tone(BUZZER_PIN, 2000, 100);
  }
}

void btnEnterOnEventChange(bool state)
{
  if (!state)
  {
    Serial.println("ENTER");
    Serial1.println("$BTN:ENTER#");
    tone(BUZZER_PIN, 2000, 100);
  }
}

void ledMesOnEventChange(bool state)
{
  if (state)
  {
    Serial.println("LED MES ON");
    Serial1.println("$LED:MES:ON#");
  }
  else
  {
    Serial.println("LED MES OFF");
    Serial1.println("$LED:MES:OFF#");
  }
}

void ledPwrOnEventChange(bool state)
{
  if (state)
  {
    Serial.println("LED PWR ON");
    Serial1.println("$LED:PWR:ON#");
  }
  else
  {
    Serial.println("LED PWR OFF");
    Serial1.println("$LED:PWR:OFF#");
  }
}

void ledRedOnEventChange(bool state)
{
  if (state)
  {
    Serial.println("LED RED ON");
    Serial1.println("$LED:RED:ON#");
  }
  else
  {
    Serial.println("LED RED OFF");
    Serial1.println("$LED:RED:OFF#");
  }
}

void ledGreenOnEventChange(bool state)
{
  if (state)
  {
    Serial.println("LED GREEN ON");
    Serial1.println("$LED:GREEN:ON#");
  }
  else
  {
    Serial.println("LED GREEN OFF");
    Serial1.println("$LED:GREEN:OFF#");
  }
}

void ledBlueOnEventChange(bool state)
{
  if (state)
  {
    Serial.println("LED BLUE ON");
    Serial1.println("$LED:BLUE:ON#");
  }
  else
  {
    Serial.println("LED BLUE OFF");
    Serial1.println("$LED:BLUE:OFF#");
  }
}

void ledLogJigOnEventChange(bool state)
{
  if (state)
  {
    Serial.println("LED LOG JIG ON");
    Serial1.println("$LED:LOGJIG:ON#");
  }
  else
  {
    Serial.println("LED LOG JIG OFF");
    Serial1.println("$LED:LOGJIG:OFF#");
  }
}

void LED_Controls(int _mode)
{
  if (_mode == 0)
  {
    // ledRed.off();
    // ledGreen.off();
    // ledBlue.off();

    // relayGreen.off();
    // relayOrange.on();
    // relayRed.off();
    // relayAram.off();
  }
  else if (_mode == 1)
  {
    // ledRed.on();
    // ledGreen.off();
    // ledBlue.off();

    // relayGreen.off();
    // relayOrange.off();
    // relayRed.on();
    // relayAram.on();
  }
  else if (_mode == 2)
  {
    // ledRed.off();
    // ledGreen.on();
    // ledBlue.off();

    // relayGreen.on();
    // relayOrange.off();
    // relayRed.off();
  }
  else if (_mode == 3)
  {
    // ledRed.off();
    // ledGreen.off();
    // ledBlue.on();
  }
}