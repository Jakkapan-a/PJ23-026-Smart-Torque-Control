#include <TcBUTTON.h>
#include <TcPINOUT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Keyboard.h"


// // -------------------- INPUT -------------------- //
#define BTN_START 11
void btnStartOnEventChange(bool state);
TcBUTTON btnStart(BTN_START, false);

#define BTN_STOP 12
void btnStopOnEventChange(bool state);
TcBUTTON btnStop(BTN_STOP, false);

#define BTN_ESC_PIN A0
void btnEscOnEventChange(bool state);
TcBUTTON btnEsc(BTN_ESC_PIN, false);

#define BTN_UP_PIN A1
void btnUpOnEventChange(bool state);
TcBUTTON btnUp(BTN_UP_PIN, false);

#define BTN_DOWN_PIN A2
void btnDownOnEventChange(bool state);
TcBUTTON btnDown(BTN_DOWN_PIN, false);

#define BTN_ENTER_PIN A3
void btnEnterOnEventChange(bool state);
TcBUTTON btnEnter(BTN_ENTER_PIN, false);

#define BUZZER_PIN 6
uint8_t passToneCount = 0;
uint32_t lastTimeTonePASS = 0;

uint8_t ngToneCount = 0;
uint32_t lastTimeToneNG = 0;

#define BUFFER_SIZE_DATA 255
// -------------------- LCD -------------------- //
LiquidCrystal_I2C lcd(0x27, 16, 2);
// -------------------- SERIAL  -------------------- //
boolean startReceived = false;
boolean endReceived = false;

const char startChar = '$';
const char endChar = '#';
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
    if(inChar == startByte1){
      startReceivedByte1 = true;
      inputByteLength1 = 0;
    }else if(startReceivedByte1 && inChar == endByte1){
      endReceivedByte1 = true;
    }else if(startReceivedByte1){
      if(inputByteLength1 < BUFFER_SIZE_DATA - 1){
        inputByte1[inputByteLength1++] = inChar;
      }else{
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
void receiveEvent(int howMany) {
  // while (1 < Wire.available()) { // Loop through all but the last
  //   char c = Wire.read(); // Receive byte as a character
  //   Serial.print(c);      // Print the character to the serial monitor
  // }
  // char x = Wire.read();    // Receive byte as an integer
  // Serial.println(x);      // Print the integer to the serial monitor
  while (Wire.available()) { // loop through all but the last
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

void requestEvent() {
  Wire.write("hello "); // Respond with a message of 6 characters
}
 
void setup() {

  lcd.begin();
  lcd.clear();
  lcd.backlight();
  Wire.begin(0x74); // Set the Arduino I2C address to 0x04
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
  updateLCD("Starting...", "Please wait....");
  btnStart.OnEventChange(btnStartOnEventChange);
  btnStop.OnEventChange(btnStopOnEventChange);
  btnEsc.OnEventChange(btnEscOnEventChange);
  btnUp.OnEventChange(btnUpOnEventChange);
  btnDown.OnEventChange(btnDownOnEventChange);
  btnEnter.OnEventChange(btnEnterOnEventChange);
  passToneCount = 2;
  delay(1000);
  Serial.print("START");
  Serial1.println("$ON:START#");
}
uint32_t previousMillis = 0;
uint32_t i = 0;
void loop() 
{
  //_serialEvent();
  //_serialEvent1();
  manageSerial();
  manageSerial1();
  manageSerialI2c();
  manageByteSerial1();

  btnStart.update();
  btnStop.update();
  btnEsc.update();
  btnUp.update();
  btnDown.update();
  btnEnter.update();

  uint32_t currentMillis = millis();
  if (currentMillis - previousMillis >= 1000)
  {
    previousMillis = currentMillis;

    // Serial.print("$Hello ");
    // Serial.print(i);
    // Serial.println("#");
    // Serial1.print("$Hello ");
    // Serial1.print(i);
    // Serial1.println("#");
    i++;
  }else if(currentMillis < previousMillis){
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

void manageByteSerial1(){
  if(startReceivedByte1 && endReceivedByte1){
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


void parseData(String dataInput) {
  dataInput.trim();
  if (dataInput.indexOf("KBD_K:") != -1) {
    String serialData = extractData(dataInput , "KBD_K:");
    // Serial.print("DataEx: ");
    // Serial.println(serialData);
    if(serialData.length() > 0){
      for (int i = 0; i < serialData.length(); i++) {
        // String str = serialData[i].toUpperCase();
        Keyboard.print(serialData[i]);
        // delay(1);
      }
      Keyboard.press(KEY_RETURN);
      // delay(1);
      Keyboard.releaseAll();
    }
 
  } else if (dataInput.indexOf("LCD:") != -1) {
    String serialDataLine1 = extractData(dataInput, "line1:");
    String serialDataLine2 = extractData(dataInput, "line2:");
    updateLCD(serialDataLine1, serialDataLine2);
  }
}

String extractData(String dataInput, String key) {
  int keyIndex = dataInput.indexOf(key);  // Find the position of the key
  if (keyIndex == -1) {
    return "";  // Return 0 if key not found
  }

 int startIndex = keyIndex + key.length();  // Start index for the number
  int endIndex = dataInput.indexOf(",", startIndex);  // Find the next comma after the key
  if (endIndex == -1) {
    endIndex = dataInput.length();  // If no comma, assume end of string
  }

  String valueStr = dataInput.substring(startIndex, endIndex);  // Extract the substring
  return valueStr;  // Return the extracted string
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

void btnStartOnEventChange(bool state)
{
  if (!state)
  {
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