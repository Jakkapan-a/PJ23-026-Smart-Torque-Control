#include <TcBUTTON.h>
#include <TcPINOUT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>

// SD Card CS
#define SD_CS 53
// -------------------- LCD -------------------- //
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- INPUT -------------------- //
#define BTN_START 11
void btnStartOnEventChange(bool state);
TcBUTTON btnStart(BTN_START, false);

#define BTN_STOP 12
void btnStopOnEventChange(bool state);
TcBUTTON btnStop(BTN_STOP, false);

#define BTN_ESC_PIN 35
void btnEscOnEventChange(bool state);
TcBUTTON btnEsc(BTN_ESC_PIN, false);

#define BTN_UP_PIN 36
void btnUpOnEventChange(bool state);
TcBUTTON btnUp(BTN_UP_PIN, false);

#define BTN_DOWN_PIN 37
void btnDownOnEventChange(bool state);
TcBUTTON btnDown(BTN_DOWN_PIN, false);

#define BTN_ENTER_PIN 38
void btnEnterOnEventChange(bool state);
TcBUTTON btnEnter(BTN_ENTER_PIN, false);

#define BTN_CENSOR_ON_ST 40
void btnCensorOnStOnEventChange(bool state);
TcBUTTON btnCensorOnSt(BTN_CENSOR_ON_ST, false);
// -------------------- OUTPUT -------------------- //
#define LED_BLUE 7
TcPINOUT ledBlue(LED_BLUE, false);
#define LED_GREEN 8
TcPINOUT ledGreen(LED_GREEN, false);
#define LED_RED 9
TcPINOUT ledRed(LED_RED, false);

#define TORQUE 10
TcPINOUT torque(TORQUE, false);

// -------------------- BUZZER -------------------- //
#define BUZZER_PIN 2

int addressModel[10] = { 1, 33, 65, 97, 129, 161, 193, 225, 257, 289 };

uint8_t passToneCount = 0;
uint8_t totalTonePASS = 2;
uint32_t lastTimeTonePASS = 0;

uint8_t ngToneCount = 0;
uint8_t totalToneNG = 15;
uint32_t lastTimeToneNG = 0;


// Define an array containing letters, numbers, and some symbols
const char letters[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                         'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                         '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };



int numChars = sizeof(letters) - 1;
int indexChar = 0;

const char lettersNumber[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
int numCharsNumber = sizeof(lettersNumber) - 1;
int indexCharNumber = 0;



int indexAddressModel = 0;
String model = "";
String modelSetName = "";
int indexModelName = 0;       // Index edit model name
uint8_t lengthNameModel = 5;  // 5 Digit


int indexNumber = 0;
uint8_t lengthNumber = 5;  // 5 Digit name

uint8_t countScrew = 0;
uint8_t countScrewMax = 1;
enum STATUS_TEST {
  PASS,
  NG,
  TESTING,
  NO_TEST,
  STOP
};
STATUS_TEST status_test = NO_TEST;

int indexMenu = 0;
int selectMenu = 0;
int selectSubMenu = 0;
int selectSubMenu1 = 0;
int selectSubMenu2 = 0;
bool isSave = false;


// -------------------- SERIAL  -------------------- //
bool startReceived = false;
bool endReceived = false;

const char startChar = '$';
const char endChar = '#';
String inputString = "";
void serialEvent() {
  while (Serial.available()) {
    byte inChar = (byte)Serial.read();
    if (inChar == startChar) {
      startReceived = true;
      inputString = "";
      inputString += (char)inChar;

    } else if (startReceived && inChar == endChar) {
      inputString += (char)inChar;
      endReceived = true;
    } else if (startReceived) {
      inputString += (char)inChar;
    }
  }
}

// -------------------- SERIAL 1 -------------------- //
bool startReceived1 = false;
bool endReceived1 = false;
const byte startChar1 = 0x02;
const byte endChar1 = 0x03;
String inputString1 = "";
void serialEvent1() {
  while (Serial1.available()) {
    byte inChar = (byte)Serial1.read();
    if (inChar == startChar1) {
      startReceived1 = true;
      inputString1 = "";
      inputString1 += String(inChar, DEC);
    } else if (startReceived1 && inChar == endChar1) {
      inputString1 += String(inChar, DEC);
      endReceived1 = true;
    } else if (startReceived1) {
      inputString1 += String(inChar, DEC);
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  lcd.begin();
  lcd.clear();

  String dot = ".";
  updateLCD("Initializing SD", dot.c_str());
  delay(100);
  while (!SD.begin(SD_CS)) {
    for (int i = 0; i < 15; i++) {
      updateLCD("Initial SD Card", dot.c_str());
      dot += ".";
      if (dot.length() > 15) {
        dot = ".";
      }
      delay(500);
    }
    delay(100);
  }

  updateLCD("SD Card", "Completed");

  // Add function of button
  btnStart.OnEventChange(btnStartOnEventChange);
  btnStart.DebounceDelay(20);  // 10ms
  btnStop.OnEventChange(btnStopOnEventChange);
  btnStop.DebounceDelay(20);  // 10ms

  btnEsc.OnEventChange(btnEscOnEventChange);
  btnUp.OnEventChange(btnUpOnEventChange);
  btnDown.OnEventChange(btnDownOnEventChange);
  btnEnter.OnEventChange(btnEnterOnEventChange);

  btnCensorOnSt.OnEventChange(btnCensorOnStOnEventChange);

  indexMenu = 0;
  uint8_t indexModel = readInt8InEEPROM(0);
}

void loop() {
  btnStart.update();
  btnStop.update();
  btnEsc.update();
  btnUp.update();
  btnDown.update();
  btnEnter.update();
  btnCensorOnSt.update();

  manageSerial();
  manageSerial1();
  unsigned long currentMillis = millis();
  mainFunction();



  ToneFun(currentMillis, lastTimeTonePASS, 200, 2000, 50, passToneCount);  //
  ToneFun(currentMillis, lastTimeToneNG, 100, 2000, 50, ngToneCount);
}
uint32_t timeComplete = 0;
uint32_t timeStart, lastDebounceTime, lastDebounceTimeMillis = 0;
boolean stateStart, stateStop, stateCensorOnStation = false;

void mainFunction() {
  if (stateStart && stateStop && stateCensorOnStation) {
    stateStart = false;
    stateStop = false;
    timeComplete = millis() - timeStart;
    countScrew++;

    Serial.println("Time complete: " + String(timeComplete));
    String data = "Time complete: " + String(timeComplete);
    updateLCD("Time complete: ", String(timeComplete).c_str());
  }

  unsigned long currentMillis = millis();
  // -------------------- Debounce 10 ms ------------------ //
  if (currentMillis - lastDebounceTime > 10) {
    if (indexMenu == 1) {
      // SettingMenu();
    }
    lastDebounceTime = currentMillis;
  } else if (currentMillis < lastDebounceTime) {  // Overflows
    lastDebounceTime = currentMillis;
  }

  // -------------------- Debounce 100 ms ------------------ //
  if (currentMillis - lastDebounceTimeMillis > 100) {
    if (indexMenu == 0) {
      // Home display
      String line1 = "MODEL : " + model;
      String line2 = "";
      if (stateStart && !stateStop && stateCensorOnStation) {
        line2 = String(countScrew) + "/" + String(countScrewMax) + "PCS," + String(timeComplete) + "ms";
      } else if (stateCensorOnStation) {
        line2 = "Ready...";
      }else {
        line2 = "----------------";
      }
      // Update LCD
      updateLCD(line1.c_str(), line2.c_str());
      if (stateCensorOnStation) {
        torque.on();
      } else {
        torque.off();
      }

    } else {
      torque.off();
    }
    lastDebounceTimeMillis = currentMillis;
  } else if (currentMillis < lastDebounceTimeMillis) {
    lastDebounceTimeMillis = currentMillis;
  }
}

void manageSerial() {
  if (startReceived && endReceived) {
    Serial.println(inputString);
    Serial.println("--------0----------");
    startReceived = false;
    endReceived = false;
    inputString = "";
  }
}
void manageSerial1() {
  if (startReceived1 && endReceived1) {
    Serial.println(inputString1);
    Serial.println("--------1----------");
    startReceived1 = false;
    endReceived1 = false;
    inputString1 = "";
    passToneCount = 1;
  }
}

char currentLine1[17] = "                ";  // 16 characters + null terminator
char currentLine2[17] = "                ";  // 16 characters + null terminator

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

void ToneFun(uint32_t _currentMillis, uint32_t &_lastTime, uint32_t _toneTime, int _toneFreq, uint8_t _dutyCycle, uint8_t &totalTone) {
  if (totalTone <= 0) {
    return;
  }
  // uint32_t currentMillis = millis();
  if (_currentMillis - _lastTime > _toneTime) {
    if (_dutyCycle > 0) {
      int p = (int)(_dutyCycle * _toneTime / 100);
      tone(BUZZER_PIN, _toneFreq, p);
      totalTone--;
    }
    _lastTime = _currentMillis;
  } else if (_currentMillis < _lastTime) {
    _lastTime = _currentMillis;
  }
}

void LED_Controls(uint8_t state = 0) {
  if (state == 0) {
    ledRed.off();
    ledGreen.off();
    ledBlue.off();
  } else if (state == 1) {
    ledRed.on();
    ledGreen.off();
    ledBlue.off();
  } else if (state == 2) {
    ledRed.off();
    ledGreen.on();
    ledBlue.off();
  } else if (state == 3) {
    ledRed.off();
    ledGreen.off();
    ledBlue.on();
  }
}

void btnStartOnEventChange(bool state) {
  stateStart = !state;
  if (stateStart) {
    Serial.println("Start");
    timeStart = millis();
    LED_Controls(3);
  } else {
    LED_Controls(0);
  }
}

void btnStopOnEventChange(bool state) {
  stateStop = !state;
  if (stateStop) {
    Serial.println("Stop");
  } else {
  }
}

void btnCensorOnStOnEventChange(bool state) {
  stateCensorOnStation = !state;
  if (!stateCensorOnStation) {
    // After stop, reset all value to default and check screw count
  }
}

void btnEscOnEventChange(bool state) {
}

void btnUpOnEventChange(bool state) {
}

void btnDownOnEventChange(bool state) {
}

void btnEnterOnEventChange(bool state) {
}


void resetStateButton() {
  // stateEsc = false;
  // stateUp = false;
  // stateDown = false;
  // stateEnter = false;
}
void updateEEPROM(int index, uint8_t data) {
  EEPROM.update(index, data);
}
void updateInt16ToEEPROM(int address, uint16_t data) {
  EEPROM.update(address, data >> 8);        // Store the higher byte
  EEPROM.update(address + 1, data & 0xFF);  // Store the lower byte
}

void updateEEPROM(int index, String data) {
  int len = data.length();
  for (int i = 0; i < len; i++) {
    EEPROM.update(index + i, data[i]);
  }
}

uint8_t readInt8InEEPROM(int index) {
  return EEPROM.read(index);
}

uint16_t readInt16CInEEPROM(int index) {
  int data = 0;
  data = EEPROM.read(index) << 8;  // Read the higher byte and shift it
  data |= EEPROM.read(index + 1);  // Read the lower byte and OR it
  return data;
}

int getAddress(int indexM, int index) {
  // int baseAddress = indexM + 10;
  int address = (indexM + 10) + (index * 2);
  return address;
}
String readEEPROM(int index, int len) {
  String data = "";
  for (int i = 0; i < len; i++) {
    data += char(EEPROM.read(index + i));
  }
  return data;
}

uint8_t getCountControls(int address) {
  int add = getAddress(address, 10);
  return readInt8InEEPROM(add);
}

uint16_t getMin(int address) {
  int add = getAddress(address, 0);
  return readInt16CInEEPROM(add);
}

uint16_t getMax(int address) {
  int add = getAddress(address, 1);
  return readInt16CInEEPROM(add);
}