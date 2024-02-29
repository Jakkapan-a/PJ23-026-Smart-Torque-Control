#include <TcBUTTON.h>
#include <TcPINOUT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include "DS1302.h"

DS1302 rtc(2, 3, 4); // RST, DAT, CLK

#define DEBUG 0
#define BUFFER_SIZE_DATA 255 // Buffer size = 255 bytes or 255 characters
#define BASE_PATH "/data"

// ------------------- Head Function -------------- //
void LED_Controls(uint8_t);

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

#define BTN_SCW_KEY 39
void btnScwKeyOnEventChange(bool state);
TcBUTTON btnScwKey(BTN_SCW_KEY, false);

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

#define RELAY_GREEN 26
TcPINOUT relayGreen(RELAY_GREEN, false);

#define RELAY_ORANGE 27
TcPINOUT relayOrange(RELAY_ORANGE, false);

#define RELAY_RED 28
TcPINOUT relayRed(RELAY_RED, false);

#define RELAY_ARAM 29
TcPINOUT relayAram(RELAY_ARAM, false);

#define TORQUE_PIN 10
void torqueOnEventChange(bool state);
TcPINOUT torque(TORQUE_PIN, false);

#define LOCK_JIG_PIN 22
void lockJigOnEventChange(bool state);
TcPINOUT lockJig(LOCK_JIG_PIN, false);

#define TICKER_UNLOCK_JIG 42
TcPINOUT tickerUnlockJig(TICKER_UNLOCK_JIG, true);

#define MES_RELAY_PIN 23
TcPINOUT mesRelay(MES_RELAY_PIN, false);

// -------------------- BUZZER -------------------- //
#define BUZZER_PIN 6
// -------------------- EEPROM -------------------- //
int addressModel[10] = {1, 33, 65, 97, 129, 161, 193, 225, 257, 289};
int indexAddressModel = 0;

// -------------------- TONE -------------------- //
uint8_t passToneCount = 0;
uint8_t totalTonePASS = 1;
uint32_t lastTimeTonePASS = 0;

uint8_t ngToneCount = 0;
uint8_t alarmsTone = 0;
uint8_t totalToneNG = 15;
uint32_t lastTimeToneNG = 0;
uint32_t lastTimeToneAlarms = 0;
uint16_t countPressUp = 0;
uint16_t countPressDown = 0;
const uint16_t pressTime = 150;

boolean statusServer = false;
boolean statusMES = false;
boolean statusETH = false;

uint8_t countDownStatusServer = 0; // Sec 13
uint8_t countDownStatusETH = 0;    // Sec 12
uint8_t countDownStatusMES = 0;    // Sec 10

#define TIME_OUT_COMMUNICATION 20 // 20 seconds

uint8_t CountUpCommunication = 0;
#define TIME_UP_COMMUNICATION 14 // 14 seconds

// 16/02/2024
#define BUFFER_DATE 20
#define BUFFER_TIME 10 // 00:00:00
char myDate[BUFFER_DATE];
char myTime[BUFFER_TIME];

// Define an array containing letters, numbers, and some symbols
const char letters[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int numChars = sizeof(letters) - 1;
int indexChar = 0;

const char lettersNumber[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int numCharsNumber = sizeof(lettersNumber) - 1;
int indexCharNumber = 0;

boolean isAllowMES, oldIsAllowMes = false;

String model = "";

String modelSetName = "";
int indexModelName = 0;      // Index edit model name
uint8_t lengthNameModel = 5; // 5 Digit

const int ID_Address = 321; // 321 - 325 = 5 byte or 5 digit
String id = "T0001";
String setId = "";
int indexID = 0;      // Index edit ID
uint8_t lengthID = 5; // 5 Digit

uint8_t IP[] = {10, 192, 13, 172};
uint8_t GATEWAY[] = {10, 192, 13, 254};
uint8_t SUBNET[] = {255, 255, 255, 0};
uint8_t MAC[] = {0x0A, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
uint8_t DNS[] = {10, 192, 13, 172};

uint8_t indexIP = 0; // Index edit IP Address, mac address, gateway, subnet, dns

const int IP_Address = 326;      // 326 - 329 = 4 byte or 4 digit
const int GATEWAY_Address = 330; // 330 - 333 = 4 byte or 4 digit
const int SUBNET_Address = 334;  // 334 - 337 = 4 byte or 4 digit
const int MAC_Address = 338;     // 338 - 343 = 6 byte or 6 digit
const int DNS_Address = 344;     // 344 - 347 = 4 byte or 4 digit

uint8_t IP_SERVER[] = {10, 192, 13, 172};
uint16_t SERVER_PORT_MQTT = 1883;

const int IP_SERVER_Address = 348;        // 348 - 351 = 4 byte or 4 digit
const int SERVER_PORT_MQTT_Address = 352; // 352 - 353 = 2 byte or 2 digit

int indexNumber = 0;
uint8_t lengthNumber = 5; // 5 Digit name

uint8_t countScrew = 0;
uint8_t countScrewMax = 2;
uint8_t setCountScrew = 0;

uint32_t timeComplete, timeStart = 0;
uint32_t stdMin = 700;
uint32_t stdMax = 2000;

uint32_t setStdMin = 700;
uint32_t setStdMax = 2000;

String _strStdMin = "";
String _strStdMax = "";

uint8_t pressUnlockJig = 0;
const uint8_t pressUnlockJigMax = 5;
uint8_t pressUnlockJigCountDown = 0;

uint32_t lastDebounceTime, lastDebounceTimeMillis, lastDebounceTimeSecond = 0;
boolean stateStart, stateStop = false;

boolean stateLockJig = false;
int countLockJig = 0;
int countUnlockJig = 0;
const int countLockJigMax = 2;

uint32_t totalNumberFile = 0;
bool isReadNumberFile = false;

enum STATUS_TEST
{
  PASS,
  NG,
  TESTING,
  NO_TEST,
  STOP
};
STATUS_TEST status_test = NO_TEST;
// -------------------- SD CARD -------------------- //
#define FILE_NAME_SIZE 11
char fileName[FILE_NAME_SIZE];
int fileIndex = 0;
// -------------------- MODEL -------------------- //
uint8_t indexSelectionModel = 0;
// -------------------- MENU -------------------- //
int indexMenu = 0; // 0: Home, 1: Setting 2: MES Serial 3: RFID
int oldIndexMenu = 0;
uint8_t countIndexMenu = 0;
#define maxMenuCount 10
String displayData = "";

int selectMenu = 0;
int selectSubMenu = 0;
int selectSubMenu1 = 0;
int selectSubMenu2 = 0;
boolean isSave = false;
// -------------------- STATE BUTTON -------------------- //
boolean stateEsc = false;
boolean stateUp = false;
boolean stateDown = false;
boolean stateEnter = false;

boolean currentStateEsc = false;
boolean currentStateUp = false;
boolean currentStateDown = false;
boolean currentStateEnter = false;
boolean stateCensorOnStation = false;

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
        startReceived = false;
        endReceived = false;
        inputStringLength = 0;
      }
    }
  }
}

// -------------------- SERIAL 1 -------------------- //
bool startReceived1 = false;
bool endReceived1 = false;
const byte startChar1 = 0x02;
const byte endChar1 = 0x03;
// String inputString1 = "";
char inputString1[BUFFER_SIZE_DATA];
byte inputByte1[BUFFER_SIZE_DATA];
int inputStringLength1 = 0;
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

    if (startReceived1)
    {
      if (inputStringLength1 < BUFFER_SIZE_DATA - 1)
      {
        inputString1[inputStringLength1++] = inChar;
        inputByte1[inputStringLength1] = inChar;
      }
      else
      {
        startReceived1 = false;
        endReceived1 = false;
        inputStringLength1 = 0;
      }
    }
  }
}
// -------------------- SERIAL 2 -------------------- //
bool startReceived2 = false;
bool endReceived2 = false;
const char startChar2 = '$';
const char endChar2 = '#';
// String inputString2 = "";
char inputString2[BUFFER_SIZE_DATA];
int inputStringLength2 = 0;
void serialEvent2()
{
  while (Serial2.available())
  {
    byte inChar = (byte)Serial2.read();
    if (inChar == startChar2)
    {
      startReceived2 = true;
      inputStringLength2 = 0;
    }
    else if (startReceived2 && inChar == endChar2)
    {
      endReceived2 = true;
    }
    else if (startReceived2)
    {
      if (inputStringLength2 < BUFFER_SIZE_DATA - 1)
      {
        inputString2[inputStringLength2++] = inChar;
      }
      else
      {
        startReceived2 = false;
        endReceived2 = false;
        inputStringLength2 = 0;
      }
    }
  }
}
// -------------------- SERIAL 3 -------------------- //
bool startReceived3 = false;
bool endReceived3 = false;
const char startChar3 = '$';
const char endChar3 = '#';
// String inputString3 = "";
char inputString3[BUFFER_SIZE_DATA];
int inputStringLength3 = 0;

void serialEvent3()
{
  while (Serial3.available())
  {
    byte inChar = (byte)Serial3.read();
    if (inChar == startChar3)
    {
      startReceived3 = true;
      inputStringLength3 = 0;
    }
    else if (startReceived3 && inChar == endChar3)
    {
      endReceived3 = true;
    }
    else if (startReceived3)
    {
      if (inputStringLength3 < BUFFER_SIZE_DATA - 1)
      {
        inputString3[inputStringLength3++] = inChar;
      }
      else
      {
        startReceived3 = false;
        endReceived3 = false;
        inputStringLength3 = 0;
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial2.begin(115200);
  Serial3.begin(115200);

  lcd.begin();
  lcd.clear();
  Serial.println("START");
  checkSDCard();
  updateLCD("SD Card", "Completed");

  tickerUnlockJig.off();
  // Add function of button
  btnStart.OnEventChange(btnStartOnEventChange);
  btnStart.DebounceDelay(10); // 20ms
  btnStop.OnEventChange(btnStopOnEventChange);
  btnStop.DebounceDelay(10); // 20ms

  btnEsc.OnEventChange(btnEscOnEventChange);
  btnEsc.DebounceDelay(20);
  btnUp.OnEventChange(btnUpOnEventChange);
  btnUp.DebounceDelay(20); // 20ms
  btnDown.OnEventChange(btnDownOnEventChange);
  btnDown.DebounceDelay(20); // 20ms
  btnEnter.OnEventChange(btnEnterOnEventChange);
  btnEnter.DebounceDelay(20); // 20ms
  btnScwKey.OnEventChange(btnScwKeyOnEventChange);
  btnCensorOnSt.OnEventChange(btnCensorOnStOnEventChange);

  torque.setCallback(torqueOnEventChange);
  lockJig.setCallback(lockJigOnEventChange);
  indexMenu = 0; // 0: Home, 1: Setting 2: MES Serial

  indexSelectionModel = readInt8InEEPROM(0);
  Serial.print("setup indexSelectionModel: ");
  Serial.println(indexSelectionModel);

  // indexModel
  model = getModelName(indexSelectionModel);
  countScrewMax = getCountControls(addressModel[indexSelectionModel]);
  // countScrewMax = getCountControls(addressModel[indexSelectionModel]);
  // get min max
  stdMin = getMin(addressModel[indexSelectionModel]);
  stdMax = getMax(addressModel[indexSelectionModel]);
  
  
  isAllowMES = false;
  LED_Controls(0);

  status_test = NO_TEST;
  // Clear
  memset(inputString, 0, BUFFER_SIZE_DATA);
  memset(inputString1, 0, BUFFER_SIZE_DATA);
  memset(inputString2, 0, BUFFER_SIZE_DATA);
  memset(inputString3, 0, BUFFER_SIZE_DATA);
  generateFileName();
}

void loop()
{
  btnStart.update();
  btnStop.update();
  btnEsc.update();
  btnUp.update();
  btnDown.update();
  btnEnter.update();
  btnScwKey.update();
  btnCensorOnSt.update();
  // -------------------- Serial -------------------- //
  manageSerial();
  manageSerial1();
  manageSerial2();
  manageSerial3();

  unsigned long currentMillis = millis();
  mainFunction();
  ToneFun(currentMillis, lastTimeTonePASS, 200, 2000, 50, passToneCount); //
  ToneFun(currentMillis, lastTimeToneNG, 100, 2000, 50, ngToneCount);
  ToneFun(currentMillis, lastTimeToneAlarms, 100, 2000, 50, alarmsTone);
}

void mainFunction()
{
  if (isAllowMES != oldIsAllowMes)
  {
    if (statusServer)
    {
      Serial3.println("$PUB=S1_ALLOW_MES:" + String(isAllowMES ? "ON" : "OFF") + "#");
    }
    oldIsAllowMes = isAllowMES;

    if(isAllowMES){
      mesRelay.on();
    }else{
      mesRelay.off();
    }
  }

  if (stateStart && stateStop && stateCensorOnStation)
  {
    stateStart = false;
    stateStop = false;
    timeComplete = millis() - timeStart;
    countScrew++;

    if (statusServer)
    {
      String data = "S1_TIME_COMPLETE:" + String(timeComplete);
      data += ",S1_SCW_COUNT:" + String(countScrew);
      data += ",S1_SCW_TOTAL:" + String(countScrewMax);
      data += ",S1_TEST:" + String(timeComplete > stdMin && timeComplete < stdMax ? "PASS" : "NG");
      Serial3.println("$PUB=" + data + "#");

      data = "TIME_COMPLETE=" + String(timeComplete);
      data += ",SCW_COUNT=" + String(countScrew);
      data += ",SCW_TOTAL=" + String(countScrewMax);
      data += ",TEST=" + String(timeComplete > stdMin && timeComplete < stdMax ? "PASS" : "NG");

      // Save data to SD Card
      appendFile(fileName, data.c_str());
    }
    else
    {
      String data = "TIME_COMPLETE=" + String(timeComplete);
      data += ",SCW_COUNT=" + String(countScrew);
      data += ",SCW_TOTAL=" + String(countScrewMax);
      data += ",TEST=" + String(timeComplete > stdMin && timeComplete < stdMax ? "PASS" : "NG");

      // Save data to SD Card
      appendFile(fileName, data.c_str());
    }
    // Check time is inside range min and max
    if (timeComplete >= stdMin && timeComplete <= stdMax)
    {
      // Check count screw
      if (countScrew >= countScrewMax)
      {
        status_test = PASS;
        passToneCount = totalTonePASS;
        // off torque
        countUnlockJig = countLockJigMax + 1;
        // MES ON
        isAllowMES = true;
        // LED ON ledGreen
        LED_Controls(2);
      }
      else
      {
        status_test = TESTING;
        // Serial.println("--------LED OFF TESTING NEXT ----------");
        isAllowMES = false;
        LED_Controls(0);
      }
      // LED on ledGreen
      passToneCount += 1;
    }
    else
    {
      // NG
      status_test = NG;
      ngToneCount = totalToneNG;
      // LED ON ledRed
      LED_Controls(1);
    }
  }

  if (ngToneCount > 0)
  {
    ngToneCount = totalToneNG;
  }

  if(isReadNumberFile == true){
    totalNumberFile = getTotalNumberOfFiles();
    isReadNumberFile = false;
  }
  if (pressUnlockJigCountDown > 0 && pressUnlockJig == 2)
  {
    tickerUnlockJig.on();
    pressUnlockJigCountDown = 0;
    pressUnlockJig = 0;
    if (statusServer)
    {
      Serial3.println("$PUB=S1_UNLOCK_BY:SW_KEY#");
    }
    else
    {

      String data = "SW_KEY=SCW_KEY";
      // Save data to SD Card
      appendFile(fileName, data.c_str());
    }
  }
  unsigned long currentMillis = millis();
  // -------------------- Debounce 10 ms ------------------ //
  if (currentMillis - lastDebounceTime > 10)
  {

    if (currentStateUp)
    {
      if (countPressUp > pressTime)
      {
        btnUpOnEventPressed();
      }
      else
      {
        countPressUp += 1;
      }
    }
    else
    {
      countPressUp = 0;
    }

    if (currentStateDown)
    {
      if (countPressDown > pressTime)
      {
        btnDownOnEventPressed();
      }
      else
      {
        countPressDown += 1;
      }
    }
    else
    {
      countPressDown = 0;
    }

    lastDebounceTime = currentMillis;
  }
  else if (currentMillis < lastDebounceTime)
  { // Overflows
    lastDebounceTime = currentMillis;
  }

  // -------------------- Debounce 100 ms ------------------ //
  if (currentMillis - lastDebounceTimeMillis > 100)
  {

    if (indexMenu == 0)
    {
      // Home display
      String line1 = "MODEL : " + model;
      String line2 = "";
      if (stateStart && !stateStop && stateCensorOnStation)
      {
        line2 = String(countScrew) + "/" + String(countScrewMax) + "PCS," + String(currentMillis - timeStart) + "ms";
      }
      else if (stateCensorOnStation)
      {
        line2 = String(countScrew) + "/" + String(countScrewMax) + "PCS," + String(timeComplete) + "ms";
      }
      else
      {
        line2 = "----------------";
      }
      // Update LCD
      updateLCD(line1.c_str(), line2.c_str());
    }
    else if (indexMenu == 1)
    {
      torque.off();
      settingMenu();
    }
    // ----- MENU 2 ----- //
    else if (indexMenu == 2)
    {
      String line1 = "Not Allow MES";
      String line2 = displayData;

      if (isAllowMES)
      {
        line1 = "MES OK";
      }
      updateLCD(line1.c_str(), line2.c_str());
    }
    // ----- MENU 3 RFID REQUEST ----- //
    else if (indexMenu == 3)
    {
      String line1 = "RFID REQUEST";
      String line2 = displayData;
      updateLCD(line1.c_str(), line2.c_str());
    }
    // ----- MENU 4 RFID RESPONSE ----- //
    else if (indexMenu == 4)
    {
      String line1 = "RFID RESPONSE";
      String line2 = displayData;
      updateLCD(line1.c_str(), line2.c_str());
    }

    if (countIndexMenu > 0)
    {
      countIndexMenu--;
      if (countIndexMenu == 0)
      {
        indexMenu = oldIndexMenu;
      }
    }

    if (pressUnlockJigCountDown > 0)
    {
      pressUnlockJigCountDown--;
      if (pressUnlockJigCountDown <= 0)
      {
        pressUnlockJig = 0;
      }
    }
    stateButtonPressed();
    resetStateButton();
    lastDebounceTimeMillis = currentMillis;
  }
  else if (currentMillis < lastDebounceTimeMillis)
  {
    lastDebounceTimeMillis = currentMillis;
  }

  // -------------------- Debounce 1000 ms ------------------ //
  if (currentMillis - lastDebounceTimeSecond > 1000)
  {
    clockDate();
    if(!stateStart){
      checkSDCard();
    }
    // reSetupETHInfo();
    CountUpCommunication++;
    if (CountUpCommunication > TIME_UP_COMMUNICATION)
    {
      CountUpCommunication = 0;
    }
    else if (CountUpCommunication == 1)
    {
      // Serial.println("Call status MES");
      Serial2.println("$STATUS:ASK#");
    }
    else if (CountUpCommunication == 2)
    {
      // Serial.println("Call status ETH");
      Serial3.println("$STATUS_ETH:ASK#");
    }
    else if (CountUpCommunication == 3)
    {
      // Serial.println("Call status SERVER");
      Serial3.println("$STATUS_SERVER:ASK#");
    }

    if (countDownStatusServer > 0)
    {
      countDownStatusServer--;
      if (countDownStatusServer == 0)
      {
        statusServer = false;
      }
      else
      {
        statusServer = true;
      }
    }

    if (countDownStatusETH > 0)
    {
      countDownStatusETH--;
      if (countDownStatusETH == 0)
      {
        statusETH = false;
      }
      else
      {
        statusETH = true;
      }
    }

    if (countDownStatusMES > 0)
    {
      countDownStatusMES--;
      if (countDownStatusMES == 0)
      {
        statusMES = false;
      }
      else
      {
        statusMES = true;
      }
    }

    // ----------- LOCK JIG ------------ //
    if (countLockJig > 0)
    {
      countLockJig--;
      if (countLockJig <= 0)
      {
        countLockJig = 0;
        lockJig.on();
        torque.on();
        countUnlockJig = 0; // Reset count unlock jig
      }
    }
    else if (countUnlockJig > 0)
    {
      countUnlockJig--;
      if (countUnlockJig <= 0)
      {
        countUnlockJig = 0;
        lockJig.off();
        countLockJig = 0; // Reset count lock jig
      }
    }
    lastDebounceTimeSecond = currentMillis;
  }
  else if (currentMillis < lastDebounceTimeSecond)
  {
    lastDebounceTimeSecond = currentMillis;
  }
}

void checkSDCard()
{
  // loop if SD card is not present
  String dot = ".";
  while (!SD.begin(SD_CS))
  {
    // Lcd print
    for (int i = 0; i < 15; i++)
    {
      updateLCD("Initial SD Card", dot.c_str());
      dot += ".";
      if (dot.length() > 15)
      {
        dot = ".";
      }
      delay(300);
    }
  }

  isReadNumberFile = true;
}

void clockDate()
{
  char *p = rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN, '/');
  size_t len = strlen(p);
  for (size_t i = 0; i < len && i < BUFFER_DATE - 1; i++)
  {
    myDate[i] = p[i];
  }
  myDate[len < BUFFER_DATE - 1 ? len : BUFFER_DATE - 1] = '\0'; // Ensure null-termination

  p = rtc.getTimeStr();
  len = strlen(p);
  for (size_t i = 0; i < len && i < BUFFER_TIME - 1; i++)
  {
    myTime[i] = p[i];
  }
  myTime[len < BUFFER_TIME - 1 ? len : BUFFER_TIME - 1] = '\0'; // Ensure null-termination

  // Serial.print("Date: ");
  // Serial.println(myDate);
  // Serial.print("Time: ");
  // Serial.println(myTime);
}

void setMenuShowErrorWithData(int index, String data)
{
  if (indexMenu != index && countIndexMenu == 0)
  {
    oldIndexMenu = indexMenu;
  }
  indexMenu = index;
  displayData = data;
  countIndexMenu = maxMenuCount;
}

void manageSerial()
{
  if (startReceived && endReceived)
  {
    Serial.println(inputString);
    parseData(inputString);
    Serial.println("--------0----------");
    startReceived = false;
    endReceived = false;
    // Clear
    memset(inputString, 0, BUFFER_SIZE_DATA);
    inputStringLength1 = 0;
  }
}

void manageSerial1()
{
  if (startReceived1 && endReceived1)
  {
    // Serial.println(inputString1,DEC);
    // byte inputByte1[BUFFER_SIZE_DATA]

    char hexStr[(inputStringLength1 * 2) + 1];
    memset(hexStr, 0, sizeof(hexStr)); // Clear hexStr

    for (int i = 0; i < inputStringLength1; i++)
    {
      Serial.print(inputByte1[i], HEX);
      Serial.print(" ");
      sprintf(&hexStr[i * 2], "%02X", inputByte1[i]);
    }

    if (!statusServer)
    {
      setMenuShowErrorWithData(3, "Server fail!");
      alarmsTone = 10;
    }
    else
    {
      setMenuShowErrorWithData(3, hexStr);
      Serial3.print("$PUB=S1_RFID:");
      Serial3.print(hexStr);
      Serial3.println("#");
    }

    // Serial.println("--------1----------");
    startReceived1 = false;
    endReceived1 = false;
    memset(inputString1, 0, BUFFER_SIZE_DATA);
    inputStringLength1 = 0;
    passToneCount += 1;
    // Clear inputByte1 buffer
    memset(inputByte1, 0, BUFFER_SIZE_DATA);
  }
}

void manageSerial2()
{
  if (startReceived2 && endReceived2)
  {
    // Serial.println(inputString2);
    parseData(inputString2);
    // Serial.println("--------2----------");
    startReceived2 = false;
    endReceived2 = false;
    memset(inputString2, 0, BUFFER_SIZE_DATA);
    inputStringLength2 = 0;
  }
}

void manageSerial3()
{
  if (startReceived3 && endReceived3)
  {
    // Serial.println(inputString3);
    parseData(inputString3); // ETH
    // Serial.println("--------3----------");
    startReceived3 = false;
    endReceived3 = false;
    memset(inputString3, 0, BUFFER_SIZE_DATA);
    inputStringLength3 = 0;
  }
}

void parseData(String data)
{
  data.trim();
  Serial.println("Rec :" + data);
  if (data.indexOf("SERIAL:") != -1)
  {
    // Send data to MES
    String serialData = extractData(data, "SERIAL:");
    setMenuShowErrorWithData(2, serialData);
    if (isAllowMES == true)
    {
      Serial.println("Send to MES: " + data);
      Serial2.println("$REV" + data + "#");

      if (statusServer)
      {
        Serial3.println("$PUB=S1_SERIAL:" + serialData + "#");
      }
      passToneCount = 1;
    }
    else
    {
      alarmsTone = 15;
      Serial.println("Not allow send to MES: " + data);
    }
    // Save serial data to SD Card
  }
  else if (data.indexOf("STATUS_MES:") != -1)
  {
    // Send data to MES
    String extract = extractData(data, "STATUS_MES:");
    if (extract == "OK")
    {
      statusMES = true;
      countDownStatusMES = TIME_OUT_COMMUNICATION;
    }
    else
    {
      statusMES = false;
    }
    // Response to master
  }
  else if (data.indexOf("STATUS_SERVER:") != -1)
  {
    // Send data to MES
    String extract = extractData(data, "STATUS_SERVER:");
    if (extract == "OK")
    {
      statusServer = true;
      countDownStatusServer = TIME_OUT_COMMUNICATION;
    }
    else
    {
      statusServer = false;
    }
    // Response to master
  }
  else if (data.indexOf("STATUS_ETH:") != -1)
  {
    // Send data to MES
    // countDownStatusETH = TIME_OUT_COMMUNICATION;
    String extract = extractData(data, "STATUS_ETH:");
    if (extract == "OK")
    {
      statusETH = true;
      countDownStatusETH = TIME_OUT_COMMUNICATION;
    }
    else
    {
      statusETH = false;
    }
    // Response to master
  }
  else if (data.indexOf("S1_RFID:") != -1)
  {
    String rfidData = extractData(data, "S1_RFID:");
    if (rfidData == "OK")
    {
      setMenuShowErrorWithData(4, "PASS");
      countLockJig = countLockJigMax;
      passToneCount += 1;
      btnScwKeyOnEventChange(false); // Unlock jig
    }
    else if (rfidData == "NOT" || rfidData == "404")
    {
      setMenuShowErrorWithData(4, "Not allow");
      alarmsTone = 15;
    }
  }
}

String extractData(String data, String key)
{
  int keyIndex = data.indexOf(key); // Find the position of the key
  if (keyIndex == -1)
  {
    return ""; // Return 0 if key not found
  }

  int startIndex = keyIndex + key.length();     // Start index for the number
  int endIndex = data.indexOf(",", startIndex); // Find the next comma after the key
  if (endIndex == -1)
  {
    endIndex = data.length(); // If no comma, assume end of string
  }

  String valueStr = data.substring(startIndex, endIndex); // Extract the substring
  return valueStr;                                        // Convert to float and return
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

void LED_Controls(uint8_t state = 0)
{
  if (state == 0)
  {
    ledRed.off();
    ledGreen.off();
    ledBlue.off();

    relayGreen.off();
    relayOrange.on();
    relayRed.off();
    relayAram.off();
  }
  else if (state == 1)
  {
    ledRed.on();
    ledGreen.off();
    ledBlue.off();

    relayGreen.off();
    relayOrange.off();
    relayRed.on();
    relayAram.on();
  }
  else if (state == 2)
  {
    ledRed.off();
    ledGreen.on();
    ledBlue.off();

    relayGreen.on();
    relayOrange.off();
    relayRed.off();
  }
  else if (state == 3)
  {
    ledRed.off();
    ledGreen.off();
    ledBlue.on();
  }
}

void btnStartOnEventChange(bool state)
{
  stateStart = !state;
  if (stateStart)
  {
    timeStart = millis(); // Stamp time start
    // Serial.print("Start: ");
    // Serial.println(timeStart);
    LED_Controls(3);
  }
  else if (status_test == TESTING)
  {
    // Serial.println("--------LED OFF STD ----------");
    LED_Controls(0);
  }
  else if (status_test == PASS || status_test == NG || countScrew >= countScrewMax)
  {
    torque.off();
  }

  if (stateStart)
  {
    if (statusServer)
    {
      Serial3.println("$PUB=S1_T_START:ON#");
    }
    else
    {
      String data = "START=" + String(timeStart);
      // Save data to SD Card
      appendFile(fileName, data.c_str());
    }
  }
  else
  {
    if (statusServer)
    {
      Serial3.println("$PUB=S1_T_START:OFF#");
    }
  }
}

void btnStopOnEventChange(bool state)
{
  stateStop = !state;
  if (stateStop)
  {
     String data = "STOP=" + String(millis());
      // Save data to SD Card
      appendFile(fileName, data.c_str());
    if (statusServer)
    {
      Serial3.println("$PUB=S1_T_STOP:ON#");
    }
  }
  else
  {
    if (statusServer)
    {
      Serial3.println("$PUB=S1_T_STOP:OFF#");
    }
  }
}

void btnCensorOnStOnEventChange(bool state)
{
  stateCensorOnStation = !state;
  if (!stateCensorOnStation)
  {
    // After stop, reset all value to default and check screw count
    countScrew = 0;
    status_test = NO_TEST;
    ngToneCount = 0; // Reset ng tone
    timeComplete = 0;
    LED_Controls(0);
    // Serial.println("--------LED OFF SENSOR ----------");
    lockJig.off();
    torque.off();
    countLockJig = 0;

    if (statusServer)
    {
      Serial3.println("$PUB=S1_STATION:OFF#");
    }
  }
  else
  {
    countLockJig = countLockJigMax;
    lastDebounceTimeSecond = millis();
    status_test = TESTING;

    // GenerateFileName
    generateFileName();

    // Serial.print("File name: ");
    // Serial.println(fileName);

    // Serial.print("Decode file name: ");
    // Serial.println(decodeFileName(fileName));

    if (statusServer)
    {
      // Serial3.println("$PUB=S1_STATION:ON#");
      // Serial3.println("$PUB=S1_MODEL:"+model+"#");
      // Serial3.println("$PUB=S1_STD:"+String(stdMin)+"-"+String(stdMax)+"#");
      // Serial3.println("$PUB=S1_FILE_ID:"+String(fileName)+"#");

      String data = "S1_STATION:ON";
      data += ",S1_MODEL:" + model;
      data += ",S1_STD:" + String(stdMin) + "-" + String(stdMax);
      data += ",S1_FILE_ID:" + String(fileName);
      Serial3.println("$PUB=" + data + "#");
    }

    String data = "MODEL=" + model;
    data += "\nID=" + id;
    // data += "\nIP=" + String(IP[0]) + "." + String(IP[1]) + "." + String(IP[2]) + "." + String(IP[3]);
    // appendFile
    appendFile(fileName, data.c_str());

    data = "DATE_TIME=" + String(myDate) + " " + String(myTime);
    data += "\nSTD_MIN=" + String(stdMin);
    data += "\nSTD_MAX=" + String(stdMax);
    data += "\nCOUNT=" + String(countScrewMax);

    appendFile(fileName, data.c_str());
  }
  // MES OFF
  isAllowMES = false;
}

void btnScwKeyOnEventChange(bool state)
{
  if (!state)
  {
    lockJig.off();
    torque.off();
    countLockJig = 0;
    countUnlockJig = 0;
    ngToneCount = 0; // Reset ng tone
    // MES ON
    isAllowMES = true;
    tickerUnlockJig.off();
  }
}

void resetStateButton()
{
  stateEsc = false;
  stateUp = false;
  stateDown = false;
  stateEnter = false;
}

// -------------------- EEPROM -------------------- //
void updateEEPROM(int index, uint8_t data)
{
  EEPROM.update(index, data);
}

void updateInt16ToEEPROM(int address, uint16_t data)
{
  EEPROM.update(address, data >> 8);       // Store the higher byte
  EEPROM.update(address + 1, data & 0xFF); // Store the lower byte
}

void updateEEPROM(int index, String data)
{
  int len = data.length();
  for (int i = 0; i < len; i++)
  {
    EEPROM.update(index + i, data[i]);
  }
}

uint8_t readInt8InEEPROM(int index)
{
  return EEPROM.read(index);
}

uint16_t readInt16CInEEPROM(int index)
{
  int data = 0;
  data = EEPROM.read(index) << 8; // Read the higher byte and shift it
  data |= EEPROM.read(index + 1); // Read the lower byte and OR it
  return data;
}

int getAddress(int indexM, int index)
{
  // int baseAddress = indexM + 10;
  int address = (indexM + 10) + (index * 2);
  return address;
}
String readEEPROM(int index, int len)
{
  String data = "";
  for (int i = 0; i < len; i++)
  {
    data += char(EEPROM.read(index + i));
  }
  return data;
}

uint8_t getCountControls(int address)
{
  int add = getAddress(address, 10);
  return readInt8InEEPROM(add);
}

uint16_t getMin(int address)
{
  int add = getAddress(address, 0);
  return readInt16CInEEPROM(add);
}

uint16_t getMax(int address)
{
  int add = getAddress(address, 1);
  return readInt16CInEEPROM(add);
}

String getID(int address)
{
  return readEEPROM(address, 5);
}

String getModelName(int index)
{
  if (index < 0 || index > 9)
  {
    return "";
  }
  return readEEPROM(addressModel[index], lengthNameModel);
}

void writeID(int address, String data)
{
  updateEEPROM(address, data);
}

void getIP(int address, uint8_t (&ip)[4])
{
  for (int i = 0; i < 4; i++)
  {
    ip[i] = readInt8InEEPROM(address + i);
  }
}

void getMac(int address, uint8_t (&mac)[6])
{
  for (int i = 0; i < 6; i++)
  {
    mac[i] = readInt8InEEPROM(address + i);
  }
}

// -------------------- BUTTON EVENT -------------------- //
void btnEscOnEventChange(bool state)
{
  currentStateEsc = !state;
  if (!state)
  {
    stateEsc = true;
    Serial.println("ESC");
    tone(BUZZER_PIN, 2000, 100);
    lastDebounceTimeMillis = millis();
  }
}

String ConvertNumberToString(uint16_t number)
{
  String str = "";
  if (number < 10)
  {
    str = "0000" + String(number);
  }
  else if (number < 100)
  {
    str = "000" + String(number);
  }
  else if (number < 1000)
  {
    str = "00" + String(number);
  }
  else if (number < 10000)
  {
    str = "0" + String(number);
  }
  else
  {
    str = String(number);
  }
  return str;
}

uint16_t ConvertStringToNumber(String str)
{
  return (uint16_t)str.toInt();
}

// -------------------- BUTTON -------------------- //
void btnUpOnEventChange(bool state)
{
  currentStateUp = !state;
  if (!state)
  {
    stateUp = true;
    // Serial.println("UP");
    tone(BUZZER_PIN, 2000, 100);
    lastDebounceTimeMillis = millis();
  }
}

void btnDownOnEventChange(bool state)
{
  currentStateDown = !state;
  if (!state)
  {
    stateDown = true;
    // Serial.println("DOWN");
    tone(BUZZER_PIN, 2000, 100);
    lastDebounceTimeMillis = millis();
  }
}

void btnEnterOnEventChange(bool state)
{
  currentStateEnter = !state;
  if (!state)
  {
    stateEnter = true;
    // Serial.println("ENTER");
    tone(BUZZER_PIN, 2000, 100);
    lastDebounceTimeMillis = millis();
  }
}

void stateButtonPressed()
{
  // 0 0 0 0
  if (!stateEsc && !stateUp && !stateDown && !stateEnter)
  {
    // not pressed
    return;
  }
  else
    // 1 1 1 0
    if (stateEsc && stateUp && stateDown && !stateEnter)
    {
      if (pressUnlockJig == 0)
      {
        pressUnlockJig = 1;
        pressUnlockJigCountDown = pressUnlockJigMax;
      }
    }
    else
      // 1 1 0 0
      if (stateEsc && stateUp && !stateDown && !stateEnter)
      {
        if (pressUnlockJig == 1)
        {
          pressUnlockJig = 2;
        }
      }
      else
        // 0 0 0 1
        if (!stateEsc && !stateUp && !stateDown && stateEnter)
        {
          btnEnterOnEventPressed();
        }
        else
          // 0 0 1 0
          if (!stateEsc && !stateUp && stateDown && !stateEnter)
          {
            btnDownOnEventPressed();
          }
          else
            // 0 1 0 0
            if (!stateEsc && stateUp && !stateDown && !stateEnter)
            {
              btnUpOnEventPressed();
            }
            else
              // 0 1 1 0
              if (!stateEsc && stateUp && stateDown && !stateEnter)
              {
                // btnUpOnEventPressed();
                btnUpDownOnEventPressed();
                // Serial.println("UP DOWN");
              }
              else
                // 1 0 0 0
                if (stateEsc && !stateUp && !stateDown && !stateEnter)
                {
                  btnEscOnEventPressed();
                }

#if 0
  Serial.print("indexMenu: ");
  Serial.println(indexMenu);
  Serial.print("selectMenu: ");
  Serial.println(selectMenu);
  Serial.print("selectSubMenu: ");
  Serial.println(selectSubMenu);
  Serial.print("selectSubMenu1: ");
  Serial.println(selectSubMenu1);
  Serial.print("selectSubMenu2: ");
  Serial.println(selectSubMenu2);
  Serial.print("indexModelName: ");
  Serial.println(indexModelName);
  Serial.print("indexChar: ");
  Serial.println(indexChar);
  Serial.print("indexCharNumber: ");
  Serial.println(indexCharNumber);
  Serial.print("indexNumber: ");
  Serial.println(indexNumber);
  Serial.println(" --------------------- ");
#endif
}

void btnEscOnEventPressed()
{
  if (selectSubMenu2 > 0)
  {
    selectSubMenu2 = 0;
  }
  else if (selectSubMenu1 > 0)
  {
    selectSubMenu1 = 0;
  }
  else if (selectSubMenu > 0)
  {
    selectSubMenu = 0;
  }
  else if (selectMenu > 0)
  {
    selectMenu = 0;
  }
  else if (indexMenu == 0)
  {
    indexMenu = 1;
  }
  else if (indexMenu == 1)
  {
    indexMenu = 0;
    if (stateCensorOnStation && status_test != NG && !isAllowMES)
    {
      torque.on();
    }
  }
  else if (indexMenu == 2)
  {
    indexMenu = 0;
  }
  lcd.noBlink();
  lcd.noCursor();
}

void btnUpOnEventPressed()
{
  if (selectSubMenu2 > 0)
  {
    // MIN MAX
    if (selectMenu == 1 && (selectSubMenu1 == 3 || selectSubMenu1 == 4))
    {
      indexCharNumber++;
      if (indexCharNumber > numCharsNumber)
      {
        indexCharNumber = 0;
      }

      if (indexNumber == 0 && indexCharNumber > 5)
      {
        indexCharNumber = 0;
      }
    }
    else
      // SCW_count
      if (selectMenu == 1 && selectSubMenu1 == 2)
      {
        setCountScrew++;
        if (setCountScrew > 50)
        {
          setCountScrew = 1;
        }
      }
      // Model Name
      else if (selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        indexChar++;
        if (indexChar > numChars)
        {
          indexChar = 0;
        }
      }
      // SET ID
      else if (selectMenu == 2 && selectSubMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        indexChar++;
        if (indexChar > numChars)
        {
          indexChar = 0;
        }
      }
      // SET IP
      else if (selectMenu == 2 && selectSubMenu == 2 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        if (IP[indexIP] == 255)
        {
          IP[indexIP] = 0;
        }
        else
        {
          IP[indexIP]++;
        }
      }
      // SET MAC
      else if (selectMenu == 2 && selectSubMenu == 3 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        if (MAC[indexIP] == 255)
        {
          MAC[indexIP] = 0;
        }
        else
        {
          MAC[indexIP]++;
        }
      }
      // SET GATEWAY
      else if (selectMenu == 2 && selectSubMenu == 4 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        if (GATEWAY[indexIP] == 255)
        {
          GATEWAY[indexIP] = 0;
        }
        else
        {
          GATEWAY[indexIP]++;
        }
      }
      // SET SUBNET
      else if (selectMenu == 2 && selectSubMenu == 5 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        if (SUBNET[indexIP] == 255)
        {
          SUBNET[indexIP] = 0;
        }
        else
        {
          SUBNET[indexIP]++;
        }
      }
      // SET DNS
      else if (selectMenu == 2 && selectSubMenu == 6 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        if (DNS[indexIP] == 255)
        {
          DNS[indexIP] = 0;
        }
        else
        {
          DNS[indexIP]++;
        }
      }
      // SET IP SERVER
      else if (selectMenu == 2 && selectSubMenu == 7 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        if (IP_SERVER[indexIP] == 255)
        {
          IP_SERVER[indexIP] = 0;
        }
        else
        {
          IP_SERVER[indexIP]++;
        }
      }

      // SET PORT
      else if (selectMenu == 2 && selectSubMenu == 8 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
      {
        indexCharNumber++;
        if (indexCharNumber > numCharsNumber)
        {
          indexCharNumber = 0;
        }
        if (indexNumber == 0 && indexCharNumber > 5)
        {
          indexCharNumber = 0;
        }
      }
  }
  else if (selectSubMenu1 > 0)
  {
    selectSubMenu1--;
    if (selectSubMenu1 < 1)
    {
      selectSubMenu1 = 5;
    }
  }
  else if (selectSubMenu > 0)
  {
    selectSubMenu--;
    if (selectMenu == 0 || selectMenu == 1)
    {
      if (selectSubMenu < 1)
      {
        selectSubMenu = 10;
      }
    }

    if (selectMenu == 2 && selectSubMenu < 1)
    {
      selectSubMenu = 8;
    }
  }
  else if (indexMenu > 0)
  {
    selectMenu--;
    // if (selectMenu < 0) {
    //   selectMenu = 2;
    // }
  }
}

void btnDownOnEventPressed()
{
  if (selectSubMenu2 > 0)
  {
    // Count
    if (selectMenu == 1 && (selectSubMenu1 == 3 || selectSubMenu1 == 4))
    {
      indexCharNumber--;
      if (indexCharNumber < 0)
      {
        indexCharNumber = numCharsNumber;
      }

      if (indexNumber == 0 && indexCharNumber > 5)
      {
        indexCharNumber = 5;
      }
    }
    else if (selectMenu == 1 && selectSubMenu1 == 2)
    {
      setCountScrew--;
      if (setCountScrew < 1)
      {
        setCountScrew = 50;
      }
    }
    else if (selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      indexChar--;
      if (indexChar < 0)
      {
        indexChar = numChars;
      }
    }
    // SET ID
    else if (selectMenu == 2 && selectSubMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      indexChar--;
      if (indexChar < 0)
      {
        indexChar = numChars;
      }
    }
    // SET IP
    else if (selectMenu == 2 && selectSubMenu == 2 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      if (IP[indexIP] == 0)
      {
        IP[indexIP] = 255;
      }
      else
      {
        IP[indexIP]--;
      }
    }
    // SET MAC
    else if (selectMenu == 2 && selectSubMenu == 3 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      if (MAC[indexIP] == 0)
      {
        MAC[indexIP] = 255;
      }
      else
      {
        MAC[indexIP]--;
      }
    }
    // SET GATEWAY
    else if (selectMenu == 2 && selectSubMenu == 4 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      if (GATEWAY[indexIP] == 0)
      {
        GATEWAY[indexIP] = 255;
      }
      else
      {
        GATEWAY[indexIP]--;
      }
    }
    // SET SUBNET
    else if (selectMenu == 2 && selectSubMenu == 5 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      if (SUBNET[indexIP] == 0)
      {
        SUBNET[indexIP] = 255;
      }
      else
      {
        SUBNET[indexIP]--;
      }
    }
    // SET DNS
    else if (selectMenu == 2 && selectSubMenu == 6 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      if (DNS[indexIP] == 0)
      {
        DNS[indexIP] = 255;
      }
      else
      {
        DNS[indexIP]--;
      }
    }
    // SET IP SERVER
    else if (selectMenu == 2 && selectSubMenu == 7 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      if (IP_SERVER[indexIP] == 0)
      {
        IP_SERVER[indexIP] = 255;
      }
      else
      {
        IP_SERVER[indexIP]--;
      }
    }
    // SET PORT
    else if (selectMenu == 2 && selectSubMenu == 8 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      indexCharNumber--;
      if (indexCharNumber < 0)
      {
        indexCharNumber = numCharsNumber;
      }
      if (indexNumber == 0 && indexCharNumber > 5)
      {
        indexCharNumber = 5;
      }
    }
  }
  else if (selectSubMenu1 > 0)
  {
    selectSubMenu1++;
  }
  else if (selectSubMenu > 0)
  {
    selectSubMenu++;
  }
  else if (indexMenu > 0)
  {
    selectMenu++;
  }
}

void btnEnterOnEventPressed()
{
  if (indexMenu == 0)
  {
    return;
  }
  // Select model
  if (selectMenu == 0 && selectSubMenu == 0)
  {
    selectSubMenu = 1;
  }
  else if (selectMenu == 0 && selectSubMenu > 0)
  {
    isSave = true;
    // Select model
    int index = selectSubMenu - 1;
    
    // Serial.print("Enter index: ");
    // Serial.println(index);

    updateEEPROM(0, index);
    model = readEEPROM(addressModel[index], lengthNameModel);
    indexSelectionModel = index;
    countScrewMax = getCountControls(addressModel[indexSelectionModel]);
    // get min max
    stdMin = getMin(addressModel[indexSelectionModel]);
    stdMax = getMax(addressModel[indexSelectionModel]);

    //     Serial.print("Enter stdMin: ");
    // Serial.println(stdMin);

    
    //     Serial.print("Enter stdMax: ");
    // Serial.println(stdMax);

    
    // Serial.println("-----------------------");

  }
  // Setting model
  else if (selectMenu == 1 && selectSubMenu == 0)
  {
    selectSubMenu = 1;
  }
  else if (selectMenu == 1 && selectSubMenu > 0 && selectSubMenu1 == 0)
  {
    selectSubMenu1 = 1;
    setStdMin = getMin(addressModel[indexAddressModel]);
    setStdMax = getMax(addressModel[indexAddressModel]);
  }
  else if (selectMenu == 1 && selectSubMenu > 0 && selectSubMenu1 > 0)
  {
    EnterSetName();
    EnterSetCountControl();
    EnterMin();
    EnterMax();
    lcd.noBlink();
    lcd.noCursor();
  }
  // System
  else if (selectMenu == 2 && selectSubMenu == 0)
  {
    selectSubMenu = 1;
  }
  else if (selectMenu == 2 && selectSubMenu > 0 && selectSubMenu1 == 0)
  {
    selectSubMenu1 = 1;
    selectSubMenu2 = 1;

    if (selectSubMenu == 1)
    {
      // Load ID from EEPROM
      setId = getID(ID_Address);
      indexID = 0;
      resetIndex(letters, indexChar, indexID, setId);
      // Serial.print("Load ID from EEPROM: ");
      // Serial.println(setId);
    }
    else if (selectSubMenu == 2)
    {
      // Load IP from EEPROM
      getIP(IP_Address, IP);
      indexIP = 0;
      // Serial.print("Load IP from EEPROM: ");
      Serial.println(IP[0]);
      Serial.println(IP[1]);
      Serial.println(IP[2]);
      Serial.println(IP[3]);
    }
    else if (selectSubMenu == 3)
    {
      // Load MAC from EEPROM
      getMac(MAC_Address, MAC);
      indexIP = 0;
#if 0
      Serial.print("Load MAC from EEPROM: ");
      Serial.println(String(MAC[0], HEX));
      Serial.println(String(MAC[1], HEX));
      Serial.println(String(MAC[2], HEX));
      Serial.println(String(MAC[3], HEX));
      Serial.println(String(MAC[4], HEX));
      Serial.println(String(MAC[5], HEX));
#endif
    }
    // GATEWAY
    else if (selectSubMenu == 4)
    {
      // Load MAC from EEPROM
      getIP(GATEWAY_Address, GATEWAY);
      indexIP = 0;
#if 0
      Serial.print("Load GATEWAY from EEPROM: ");
      Serial.println(String(GATEWAY[0], DEC));
      Serial.println(String(GATEWAY[1], DEC));
      Serial.println(String(GATEWAY[2], DEC));
      Serial.println(String(GATEWAY[3], DEC));
#endif
    }
    // SUBNET
    else if (selectSubMenu == 5)
    {
      // Load MAC from EEPROM
      getIP(SUBNET_Address, SUBNET);
      indexIP = 0;
#if 0
      Serial.print("Load SUBNET from EEPROM: ");
      Serial.println(String(SUBNET[0], DEC));
      Serial.println(String(SUBNET[1], DEC));
      Serial.println(String(SUBNET[2], DEC));
      Serial.println(String(SUBNET[3], DEC));
#endif
    }
    // DNS
    else if (selectSubMenu == 6)
    {
      // Load from EEPROM
      getIP(DNS_Address, DNS);
      indexIP = 0;
#if 0
      Serial.print("Load DNS from EEPROM: ");
      Serial.println(String(DNS[0], DEC));
      Serial.println(String(DNS[1], DEC));
      Serial.println(String(DNS[2], DEC));
      Serial.println(String(DNS[3], DEC));
#endif
    }
    // IP SERVER
    else if (selectSubMenu == 7)
    {
      // Load from EEPROM
      getIP(IP_SERVER_Address, IP_SERVER);
      indexIP = 0;
#if 0
      Serial.print("Load IP SERVER from EEPROM: ");
      Serial.println(String(IP_SERVER[0], DEC));
      Serial.println(String(IP_SERVER[1], DEC));
      Serial.println(String(IP_SERVER[2], DEC));
      Serial.println(String(IP_SERVER[3], DEC));
#endif
    }
    // PORT
    else if (selectSubMenu == 8)
    {
      // Load from EEPROM
      SERVER_PORT_MQTT = readInt16CInEEPROM(SERVER_PORT_MQTT_Address);
      indexNumber = 0;
      resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(SERVER_PORT_MQTT));
#if 0
      Serial.print("Load PORT from EEPROM: ");
      Serial.println(SERVER_PORT_MQTT);
#endif
    }
  }
  else if (selectMenu == 2 && selectSubMenu > 0 && selectSubMenu1 > 0)
  {
    // SAVE
    EnterSetID();
    EnterSetIP();
    EnterSetMac();
    EnterSetGateway();
    EnterSetSubnet();
    EnterSetDNS();
    EnterSetIPServer();
    EnterSetPort();
  }
}

void EnterSetName()
{
  if (selectSubMenu1 != 1)
    return;
  //
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    modelSetName = readEEPROM(addressModel[indexAddressModel], lengthNameModel);
    // modelSetName = model;
    indexModelName = 0;
    // resetIndexChar();
    resetIndex(letters, indexChar, indexModelName, modelSetName);
  }
  else if (selectSubMenu2 > 0)
  {
    if (selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
    {
      indexModelName++;
      // resetIndexChar();
      resetIndex(letters, indexChar, indexModelName, modelSetName);
      if (indexModelName >= lengthNameModel)
      {
        indexModelName = 0;

        updateLCD("Save...........", "               ");
        // model = modelSetName;
        // Save to EEPROM
        updateEEPROM(addressModel[indexAddressModel], modelSetName);
        // Read EEPROM
        if (indexAddressModel == indexSelectionModel)
        {
          model = readEEPROM(addressModel[indexAddressModel], lengthNameModel);
        }
        delay(1000);
        // Update LCD
        updateLCD("Save Completed ", "               ");
        resetIndex(letters, indexChar, indexModelName, modelSetName);
        selectSubMenu2 = 0;
      }
    }
  }
}

void EnterSetCountControl()
{
  if (selectSubMenu1 != 2)
    return;

  if (selectSubMenu2 == 0)
  {
    setCountScrew = getCountControls(addressModel[indexAddressModel]);
    // Set
    selectSubMenu2 = 1;
  }
  else if (selectSubMenu2 > 0)
  {
    updateLCD("Save...........", "               ");
    // Save to EEPROM
    uint8_t address = getAddress(addressModel[indexAddressModel], 10);
    updateEEPROM(address, setCountScrew);
    if (indexAddressModel == indexSelectionModel)
    {
      UpdateCountControlsSCW();
      countScrewMax = setCountScrew;
    }
    delay(1000);
    selectSubMenu2 = 0;
    lcd.noBlink();
    lcd.noCursor();
  }
}

void EnterMin()
{
  if (selectSubMenu1 != 3)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    setStdMin = getMin(addressModel[indexAddressModel]);
    indexNumber = 0;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(setStdMin));
  }
  else if (selectSubMenu2 > 0)
  {
    indexNumber++;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(setStdMin));
    if (indexNumber >= lengthNumber)
    {
      indexNumber = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      uint8_t address = getAddress(addressModel[indexAddressModel], 0);
      updateInt16ToEEPROM(address, setStdMin);
      if (indexAddressModel == indexSelectionModel)
      {
        stdMin = setStdMin;
      }
      delay(1000);
      selectSubMenu2 = 0;
    }
  }
}

void EnterMax()
{
  if (selectSubMenu1 != 4)
    return;
  if (selectSubMenu2 == 0)
  {
    setStdMax = getMax(addressModel[indexAddressModel]);
    // Set
    selectSubMenu2 = 1;

    indexNumber = 0;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(setStdMax));
  }
  else if (selectSubMenu2 > 0)
  {
    indexNumber++;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(setStdMax));
    if (indexNumber >= lengthNumber)
    {
      indexNumber = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      uint8_t address = getAddress(addressModel[indexAddressModel], 1);
      updateInt16ToEEPROM(address, setStdMax);
      if (indexAddressModel == indexSelectionModel)
      {
        stdMax = setStdMax;
      }
      delay(1000);
      selectSubMenu2 = 0;
    }
  }
}

void EnterSetID()
{
  if (selectSubMenu != 1)
    return;
  if (selectSubMenu1 != 1)
    return;

  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    setId = getID(ID_Address);
    indexID = 0;
    resetIndex(letters, indexChar, indexID, setId);
  }
  else if (selectSubMenu2 > 0)
  {
    indexID++;
    if (indexID >= lengthID)
    {
#if DEBUG
      Serial.println("Save ID to EEPROM");
      Serial.print("ID: ");
      Serial.println(setId);
#endif

      indexID = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      writeID(ID_Address, setId);
      delay(1000);
      selectSubMenu1 = 0;
      selectSubMenu2 = 0;
      id = setId;
      lcd.noBlink();
      lcd.noCursor();
    }
    resetIndex(letters, indexChar, indexID, setId);
  }
}

void EnterSetIP()
{
  if (selectSubMenu != 2)
    return;
  if (selectSubMenu1 != 1)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    getIP(IP_Address, IP);
    indexIP = 0;
  }
  else if (selectSubMenu2 > 0)
  {
    indexIP++;
    if (indexIP >= 4)
    {
      indexIP = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      for (int i = 0; i < 4; i++)
      {
        updateEEPROM(IP_Address + i, IP[i]);
      }
      delay(1000);
      selectSubMenu1 = 0;
      selectSubMenu2 = 0;
      lcd.noBlink();
      lcd.noCursor();
    }
  }
}

void EnterSetMac()
{
  if (selectSubMenu != 3)
    return;
  if (selectSubMenu1 != 1)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    getMac(MAC_Address, MAC);
    indexIP = 0;
  }
  else if (selectSubMenu2 > 0)
  {
    indexIP++;
    if (indexIP >= 6)
    {
      indexIP = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      for (int i = 0; i < 6; i++)
      {
        updateEEPROM(MAC_Address + i, MAC[i]);
      }
      delay(1000);
      selectSubMenu1 = 0;
      selectSubMenu2 = 0;
      lcd.noBlink();
      lcd.noCursor();
    }
  }
}

void EnterSetGateway()
{
  if (selectSubMenu != 4)
    return;
  if (selectSubMenu1 != 1)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    getIP(GATEWAY_Address, GATEWAY);
    indexIP = 0;
  }
  else if (selectSubMenu2 > 0)
  {
    indexIP++;
    if (indexIP >= 4)
    {
      indexIP = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      for (int i = 0; i < 4; i++)
      {
        updateEEPROM(GATEWAY_Address + i, GATEWAY[i]);
      }
      delay(1000);
      selectSubMenu1 = 0;
      selectSubMenu2 = 0;
      lcd.noBlink();
      lcd.noCursor();
    }
  }
}

void EnterSetSubnet()
{
  if (selectSubMenu != 5)
    return;
  if (selectSubMenu1 != 1)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    getIP(SUBNET_Address, SUBNET);
    indexIP = 0;
  }
  else if (selectSubMenu2 > 0)
  {
    indexIP++;
    if (indexIP >= 4)
    {
      indexIP = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      for (int i = 0; i < 4; i++)
      {
        updateEEPROM(SUBNET_Address + i, SUBNET[i]);
      }
      delay(1000);
      selectSubMenu1 = 0;
      selectSubMenu2 = 0;
      lcd.noBlink();
      lcd.noCursor();
    }
  }
}

void EnterSetDNS()
{
  if (selectSubMenu != 6)
    return;
  if (selectSubMenu1 != 1)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    getIP(DNS_Address, DNS);
    indexIP = 0;
  }
  else if (selectSubMenu2 > 0)
  {
    indexIP++;
    if (indexIP >= 4)
    {
      indexIP = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      for (int i = 0; i < 4; i++)
      {
        updateEEPROM(DNS_Address + i, DNS[i]);
      }
      delay(1000);
      selectSubMenu1 = 0;
      selectSubMenu2 = 0;
      lcd.noBlink();
      lcd.noCursor();
    }
  }
}

void EnterSetIPServer()
{
  if (selectSubMenu != 7)
    return;
  if (selectSubMenu1 != 1)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    getIP(IP_SERVER_Address, IP_SERVER);
    indexIP = 0;
  }
  else if (selectSubMenu2 > 0)
  {
    indexIP++;
    if (indexIP >= 4)
    {
      indexIP = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      for (int i = 0; i < 4; i++)
      {
        updateEEPROM(IP_SERVER_Address + i, IP_SERVER[i]);
      }
      delay(1000);
      selectSubMenu1 = 0;
      selectSubMenu2 = 0;
      lcd.noBlink();
      lcd.noCursor();
    }
  }
}

void EnterSetPort()
{
  if (selectSubMenu != 8)
    return;
  if (selectSubMenu1 != 1)
    return;
  if (selectSubMenu2 == 0)
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    SERVER_PORT_MQTT = readInt16CInEEPROM(SERVER_PORT_MQTT_Address);
    indexNumber = 0;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(SERVER_PORT_MQTT));
  }
  else if (selectSubMenu2 > 0)
  {
    indexNumber++;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(SERVER_PORT_MQTT));
    if (indexNumber >= lengthNumber)
    {
      indexNumber = 0;
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      updateInt16ToEEPROM(SERVER_PORT_MQTT_Address, SERVER_PORT_MQTT);
      delay(1000);
      selectSubMenu2 = 0;
      lcd.noBlink();
      lcd.noCursor();
    }
  }
}

void UpdateCountControlsSCW()
{
  setCountScrew = getCountControls(addressModel[indexSelectionModel]);
}

void resetIndex(String lettersC, int &indexC, int index, String data)
{
  for (int i = 0; i < numChars; i++)
  {
    if (lettersC[i] == data[index])
    {
      indexC = i;
      break;
    }
  }
}

void btnUpDownOnEventPressed()
{
  Serial.println("UP DOWN");
  if (selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
  {
    indexModelName = 0;
    resetIndex(letters, indexChar, indexModelName, modelSetName);
  }
  else if (selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 3)
  {
    indexNumber = 0;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(setStdMin));
  }
  else if (selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 4)
  {
    indexNumber = 0;
    resetIndex(lettersNumber, indexCharNumber, indexNumber, ConvertNumberToString(setStdMax));
  }
  // SET ID
  else if (selectSubMenu2 > 0 && selectMenu == 2 && selectSubMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
  {
    indexID = 0;
    resetIndex(letters, indexChar, indexID, setId);
  }
  // SET IP
  else if (selectSubMenu2 > 0 && selectMenu == 2 && selectSubMenu == 2 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
  {
    indexIP = 0;
  }
  // SET MAC
  else if (selectSubMenu2 > 0 && selectMenu == 2 && selectSubMenu == 3 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
  {
    indexIP = 0;
  }
  // SET GATEWAY
  else if (selectSubMenu2 > 0 && selectMenu == 2 && selectSubMenu == 4 && selectSubMenu1 == 1 && selectSubMenu2 == 1)
  {
    indexIP = 0;
  }
}

// -------------------- MENU -------------------- //
void settingMenu()
{
  String line1 = "               ";
  String line2 = "               ";

  if (isSave && selectSubMenu > 0)
  {
    line1 = "SAVE-----------";
    line2 = "               ";
    updateLCD(line1, line2);
    isSave = false;
    // clear menu setting
    selectMenu = 0;
    selectSubMenu = 0;
    selectSubMenu1 = 0;
    selectSubMenu2 = 0;
    indexModelName = 0;
    indexNumber = 0;
    indexChar = 0;
    lcd.noBlink();
    lcd.noCursor();
    delay(1000);
    return;
  }
  // --------------------------------- Select model ------------------------- //
  if (selectMenu == 0 && selectSubMenu > 0)
  {
    selectModelPage(selectSubMenu, line1, line2);
  }

  // --------------------------------- Setting model ------------------------- //
  else if (selectMenu == 1 && selectSubMenu > 0)
  {
    settingModelPage(selectSubMenu, line1, line2);
  }
  // --------------------------------- System ------------------------- //
  else if (selectMenu == 2 && selectSubMenu > 0)
  {
    systemMenuPage(selectSubMenu, line1, line2);
  }
  else if (selectSubMenu == 0)
  {
    selectMenuPage(selectMenu, line1, line2);
  }
  updateLCD(line1, line2);
}

void selectMenuPage(int &_selectMenu, String &line1, String &line2)
{
  if (_selectMenu == 0)
  {
    line1 = ">SELECT MODEL";
    line2 = " SETTING MODEL";
  }
  else if (_selectMenu == 1)
  {
    line1 = " SELECT MODEL";
    line2 = ">SETTING MODEL";
  }
  else if (_selectMenu == 2)
  {
    line1 = ">SYSTEM";
    line2 = " SERVER: " + String(statusServer ? "ONLINE" : "OFFLINE"); //  STATUS: online or offline
  }
  else if (_selectMenu == 3)
  {
    line1 = " SYSTEM";
    line2 = ">SERVER: " + String(statusServer ? "ONLINE" : "OFFLINE"); //  STATUS: online or offline
  }
  else if (_selectMenu == 4)
  {
    line1 = ">ETH: " + String(statusETH ? "TRUE" : "FALSE");
    line2 = " H-USB: " + String(statusMES ? "TRUE" : "FALSE"); //  STATUS: online or offline
  }
  else if (_selectMenu == 5)
  {
    line1 = " ETH: " + String(statusETH ? "TRUE" : "FALSE");
    line2 = ">H-USB: " + String(statusMES ? "TRUE" : "FALSE"); //  STATUS: online or offline
  }
  else if (_selectMenu == 6)
  {
    line1 = ">DATE:" + String(myDate);
    line2 = " TIME:" + String(myTime);
  }
  else if (_selectMenu == 7)
  {
    line1 = " DATE:" + String(myDate);
    line2 = ">TIME:" + String(myTime);
  }
  else if (_selectMenu > 7)
  {
    _selectMenu = 0;
  }
  else if (_selectMenu < 0)
  {
    _selectMenu = 7;
  }

  else if (_selectMenu > 5)
  {
    _selectMenu = 0;
  }
  else if (_selectMenu < 0)
  {
    _selectMenu = 5;
  }
}

void selectModelPage(int &selectSubMenu, String &line1, String &line2)
{
  if (selectSubMenu == 1)
  {
    String model1 = getModelName(0);
    String model2 = getModelName(1);
    line1 = ">1.MODEL " + model1;
    line2 = " 2.MODEL " + model2;
  }
  else if (selectSubMenu == 2)
  {
    String model1 = getModelName(0);
    String model2 = getModelName(1);
    line1 = " 1.MODEL " + model1;
    line2 = ">2.MODEL " + model2;
  }
  else if (selectSubMenu == 3)
  {
    String model1 = getModelName(2);
    String model2 = getModelName(3);
    line1 = ">3.MODEL " + model1;
    line2 = " 4.MODEL " + model2;
  }
  else if (selectSubMenu == 4)
  {
    String model1 = getModelName(2);
    String model2 = getModelName(3);
    line1 = " 3.MODEL " + model1;
    line2 = ">4.MODEL " + model2;
  }
  else if (selectSubMenu == 5)
  {
    String model1 = getModelName(4);
    String model2 = getModelName(5);
    line1 = ">5.MODEL " + model1;
    line2 = " 6.MODEL " + model2;
  }
  else if (selectSubMenu == 6)
  {
    String model1 = getModelName(4);
    String model2 = getModelName(5);
    line1 = " 5.MODEL " + model1;
    line2 = ">6.MODEL " + model2;
  }
  else if (selectSubMenu == 7)
  {
    String model1 = getModelName(6);
    String model2 = getModelName(7);
    line1 = ">7.MODEL " + model1;
    line2 = " 8.MODEL " + model2;
  }
  else if (selectSubMenu == 8)
  {
    String model1 = getModelName(6);
    String model2 = getModelName(7);
    line1 = " 7.MODEL " + model1;
    line2 = ">8.MODEL " + model2;
  }
  else if (selectSubMenu == 9)
  {
    String model1 = getModelName(8);
    String model2 = getModelName(9);
    line1 = ">9.MODEL " + model1;
    line2 = " 10.MODEL " + model2;
  }
  else if (selectSubMenu == 10)
  {
    String model1 = getModelName(8);
    String model2 = getModelName(9);
    line1 = " 9.MODEL " + model1;
    line2 = ">10.MODEL " + model2;
  }
  else if (selectSubMenu > 10)
  {
    selectSubMenu = 1;
  }
  else if (selectSubMenu < 1)
  {
    selectSubMenu = 10;
  }

  indexAddressModel = selectSubMenu - 1;
}

void settingModelPage(int &selectSubMenu, String &line1, String &line2)
{
  if (selectSubMenu1 > 0)
  {
    // ---------------------  Parameter By Model ------------------ //
    // ---------------------  Name ------------------ //
    if (selectSubMenu1 == 1)
    {
      line1 = ">NAME";
      line2 = " COUNT CONTROLS";
      if (selectSubMenu2 > 0)
      {
        lcd.setCursor(indexModelName, 1);
        lcd.cursor();
        lcd.blink();
        char buf[lengthNameModel + 1];
        for (int i = 0; i < lengthNameModel + 1; i++)
        {
          buf[i] = ' ';
        }
        modelSetName.toCharArray(buf, lengthNameModel + 1);
        buf[indexModelName] = letters[indexChar];
        modelSetName = String(buf);
        line1 = "MODEL NAME:";
        line2 = modelSetName;
      }
      // --------------------- end Name ------------------ //
    }
    else if (selectSubMenu1 == 2)
    {
      line1 = " NAME";
      line2 = ">COUNT CONTROLS";
      if (selectSubMenu2 > 0)
      {
        if (selectSubMenu2 > 0)
        {
          line1 = "SCW COUNT:      ";
          line2 = String(setCountScrew) + " PCS";
        }
      }

      // --------------------- end Name ------------------ //
    }
    else if (selectSubMenu1 == 3)
    {
      line1 = ">MIN:" + String(setStdMin);
      line2 = " MAX:" + String(setStdMax);
      if (selectSubMenu2 > 0)
      {

        // indexNumber
        lcd.setCursor(indexNumber, 1);
        lcd.cursor();
        lcd.blink();

        char buf[lengthNumber + 1];
        for (int i = 0; i < lengthNumber + 1; i++)
        {
          buf[i] = ' ';
        }

        _strStdMin = ConvertNumberToString(setStdMin);

        _strStdMin.toCharArray(buf, lengthNumber + 1);

        buf[indexNumber] = lettersNumber[indexCharNumber];

        // unsigned int
        String bufString = String(buf);               // Create a String object from the char array
        setStdMin = ConvertStringToNumber(bufString); // bufString.toInt(); // Convert the String to an integer

        line1 = "SET MIN:      ";
        line2 = ConvertNumberToString(setStdMin) + " ms";
      }
    }
    else if (selectSubMenu1 == 4)
    {
      line1 = " MIN:" + String(setStdMin);
      line2 = ">MAX:" + String(setStdMax);
      if (selectSubMenu2 > 0)
      {

        // indexNumber
        lcd.setCursor(indexNumber, 1);
        lcd.cursor();
        lcd.blink();

        char buf[lengthNumber + 1];
        for (int i = 0; i < lengthNumber + 1; i++)
        {
          buf[i] = ' ';
        }

        _strStdMax = ConvertNumberToString(setStdMax);

        _strStdMax.toCharArray(buf, lengthNumber + 1);

        buf[indexNumber] = lettersNumber[indexCharNumber];

        // unsigned int
        String bufString = String(buf);               // Create a String object from the char array
        setStdMax = ConvertStringToNumber(bufString); // bufString.toInt(); // Convert the String to an integer

        line1 = "SET MAX:      ";
        line2 = ConvertNumberToString(setStdMax) + " ms";
      }
    }
    // ---------------------  End Parameter By Model ------------------ //
    else if (selectSubMenu1 > 4)
    {
      selectSubMenu1 = 1;
    }
    else if (selectSubMenu1 < 1)
    {
      selectSubMenu1 = 5;
    }
    // ---------------------  End Parameter By Model ------------------ //
  }
  else if (selectSubMenu == 1)
  {
    String model1 = getModelName(0);
    String model2 = getModelName(1);
    line1 = ">1.MODEL " + model1;
    line2 = " 2.MODEL " + model2;
  }
  else if (selectSubMenu == 2)
  {
    String model1 = getModelName(0);
    String model2 = getModelName(1);
    line1 = " 1.MODEL " + model1;
    line2 = ">2.MODEL " + model2;
  }
  else if (selectSubMenu == 3)
  {
    String model1 = getModelName(2);
    String model2 = getModelName(3);
    line1 = ">3.MODEL " + model1;
    line2 = " 4.MODEL " + model2;
  }
  else if (selectSubMenu == 4)
  {
    String model1 = getModelName(2);
    String model2 = getModelName(3);
    line1 = " 3.MODEL " + model1;
    line2 = ">4.MODEL " + model2;
  }
  else if (selectSubMenu == 5)
  {
    String model1 = getModelName(4);
    String model2 = getModelName(5);
    line1 = ">5.MODEL " + model1;
    line2 = " 6.MODEL " + model2;
  }
  else if (selectSubMenu == 6)
  {
    String model1 = getModelName(4);
    String model2 = getModelName(5);
    line1 = " 5.MODEL " + model1;
    line2 = ">6.MODEL " + model2;
  }
  else if (selectSubMenu == 7)
  {
    String model1 = getModelName(6);
    String model2 = getModelName(7);
    line1 = ">7.MODEL " + model1;
    line2 = " 8.MODEL " + model2;
  }
  else if (selectSubMenu == 8)
  {
    String model1 = getModelName(6);
    String model2 = getModelName(7);
    line1 = " 7.MODEL " + model1;
    line2 = ">8.MODEL " + model2;
  }
  else if (selectSubMenu == 9)
  {
    String model1 = getModelName(8);
    String model2 = getModelName(9);
    line1 = ">9.MODEL " + model1;
    line2 = " 10.MODEL " + model2;
  }
  else if (selectSubMenu == 10)
  {
    String model1 = getModelName(8);
    String model2 = getModelName(9);
    line1 = " 9.MODEL " + model1;
    line2 = ">10.MODEL " + model2;
  }
  else if (selectSubMenu > 10)
  {
    selectSubMenu = 1;
  }
  else if (selectSubMenu < 1)
  {
    selectSubMenu = 10;
  }
  indexAddressModel = selectSubMenu - 1;
}

void systemMenuPage(int &selectSubMenu, String &line1, String &line2)
{
  // 1. ID : 12345 : 5 char enter to set
  // 2. IP : enter to set
  // 3. Mac Address
  // 4. Gateway
  // 5. Subnet
  // 6. DNS Server
  // 7. IP Server
  // 8. Port Server
  if (selectSubMenu1 > 0)
  {
    selectSubMenu1 = 0; // reset
    //-- ID -- //
    if (selectSubMenu == 1)
    {
      if (selectSubMenu2 > 0)
      {
        lcd.setCursor(indexID, 1);
        lcd.cursor();
        lcd.blink();
        char buf[lengthID + 1];
        for (int i = 0; i < lengthID + 1; i++)
        {
          buf[i] = ' ';
        }
        setId.toCharArray(buf, lengthID + 1);
        buf[indexID] = letters[indexChar];
        setId = String(buf);
        line1 = "SET ID: ";
        line2 = setId;
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }
    // -- IP --//
    else if (selectSubMenu == 2)
    {
      if (selectSubMenu2 > 0)
      {
        uint8_t cursorIP = 0;
        indexIpCal(IP, indexIP, cursorIP);
        lcd.setCursor(cursorIP, 1);
        lcd.cursor();
        lcd.blink();
        String ipString = String(IP[0]) + "." + String(IP[1]) + "." + String(IP[2]) + "." + String(IP[3]);

        // ipString = String(buf);
        line1 = "SET IP: ";
        line2 = ipString;
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }
    // -- MAC -- //
    else if (selectSubMenu == 3)
    {
      if (selectSubMenu2 > 0)
      {

        line1 = "SET MAC Address";
        if (indexIP <= 3)
        {
          uint8_t cursorMac = 0;
          String strMac = "";
          indexMacCal(MAC, indexIP, cursorMac, strMac);
          lcd.setCursor(cursorMac + 3, 1);
          lcd.cursor();
          lcd.blink();
          // 1 = 0A:1B:2C:3D
          line2 = "1 =" + strMac;
        }
        else
        {
          uint8_t cursorMac = 0;
          String strMac = "";
          indexMacCal(MAC, indexIP, cursorMac, strMac);

          lcd.setCursor(cursorMac + 3, 1);
          lcd.cursor();
          lcd.blink();
          // 2 = 4E:5F
          line2 = "2 =" + strMac;
        }
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }
    // -- Gateway -- //
    else if (selectSubMenu == 4)
    {
      if (selectSubMenu2 > 0)
      {
        uint8_t cursorGW = 0;
        indexIpCal(GATEWAY, indexIP, cursorGW);
        lcd.setCursor(cursorGW, 1);
        lcd.blink();

        String setGateway = String(GATEWAY[0]) + "." + String(GATEWAY[1]) + "." + String(GATEWAY[2]) + "." + String(GATEWAY[3]);
        line1 = "SET GATEWAY: ";
        line2 = setGateway;
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }
    // -- Subnet -- //
    else if (selectSubMenu == 5)
    {
      if (selectSubMenu2 > 0)
      {
        uint8_t cursorSubnet = 0;
        indexIpCal(SUBNET, indexIP, cursorSubnet);
        lcd.setCursor(cursorSubnet, 1);
        lcd.blink();
        String setSubnet = String(SUBNET[0]) + "." + String(SUBNET[1]) + "." + String(SUBNET[2]) + "." + String(SUBNET[3]);
        line1 = "SET SUBNET: ";
        line2 = setSubnet;
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }
    // -- DNS -- //
    else if (selectSubMenu == 6)
    {
      if (selectSubMenu2 > 0)
      {
        uint8_t cursorDNS = 0;
        indexIpCal(DNS, indexIP, cursorDNS);
        lcd.setCursor(cursorDNS, 1);
        lcd.blink();

        String setDNS = String(DNS[0]) + "." + String(DNS[1]) + "." + String(DNS[2]) + "." + String(DNS[3]);
        line1 = "SET DNS: ";
        line2 = setDNS;
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }
    // -- IP Server -- //
    else if (selectSubMenu == 7)
    {
      if (selectSubMenu2 > 0)
      {
        uint8_t cursorIPServer = 0;
        indexIpCal(IP_SERVER, indexIP, cursorIPServer);
        lcd.setCursor(cursorIPServer, 1);
        lcd.blink();

        String setIPServer = String(IP_SERVER[0]) + "." + String(IP_SERVER[1]) + "." + String(IP_SERVER[2]) + "." + String(IP_SERVER[3]);
        line1 = "SET IP SERVER: ";
        line2 = setIPServer;
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }

    // -- Port Server -- //
    else if (selectSubMenu == 8)
    {
      if (selectSubMenu2 > 0)
      {
        lcd.setCursor(indexNumber, 1);
        lcd.cursor();
        lcd.blink();
        char buf[lengthNumber + 1];
        for (int i = 0; i < lengthNumber + 1; i++)
        {
          buf[i] = ' ';
        }
        String _strPortServer = ConvertNumberToString(SERVER_PORT_MQTT);
        _strPortServer.toCharArray(buf, lengthNumber + 1);
        buf[indexNumber] = lettersNumber[indexCharNumber];
        SERVER_PORT_MQTT = ConvertStringToNumber(String(buf));
        line1 = "SET PORT SERVER: ";
        line2 = ConvertNumberToString(SERVER_PORT_MQTT);
      }
      else
      {
        selectSubMenu1 = 0; // reset
      }
    }
  }
  else if (selectSubMenu == 1)
  {
    line1 = ">ID: " + id;
    line2 = " IP";
  }
  else if (selectSubMenu == 2)
  {
    line1 = " ID: " + id;
    line2 = ">IP";
  }
  else if (selectSubMenu == 3)
  {
    line1 = ">MAC ";
    line2 = " GATEWAY ";
  }
  else if (selectSubMenu == 4)
  {
    line1 = " MAC ";
    line2 = ">GATEWAY ";
  }
  else if (selectSubMenu == 5)
  {
    line1 = ">SUBNET ";
    line2 = " DNS ";
  }
  else if (selectSubMenu == 6)
  {
    line1 = " SUBNET ";
    line2 = ">DNS ";
  }
  else if (selectSubMenu == 7)
  {
    line1 = ">IP SERVER";
    line2 = " PORT SERVER ";
  }
  else if (selectSubMenu == 8)
  {
    line1 = " IP SERVER";
    line2 = ">PORT SERVER";
  }
  else if (selectSubMenu > 8)
  {
    selectSubMenu = 1;
  }
  else if (selectSubMenu < 1)
  {
    selectSubMenu = 8;
  }
}

void indexIpCal(uint8_t ip[], uint8_t index_input, uint8_t &index_output)
{
  //  check index_input
  // xxx.xxx.xxx.xxx
  if (index_input < 4)
  {
    //
    uint8_t sumDigits = 0;
    for (uint8_t i = 0; i <= index_input; ++i)
    {
      sumDigits += getDigit(ip[i]);
    }
    //
    uint8_t base = sumDigits + index_input;
    index_output = base - 1;
#if 0
    Serial.print("IP: ");
    Serial.print(ip[index_input]);
    Serial.print(" digit: ");
    Serial.print(getDigit(ip[index_input]));
    Serial.print(" base: ");
    Serial.print(base);
    Serial.print(" index_output: ");
    Serial.println(index_output);
#endif
  }
  else
  {
    index_output = 0;
  }
}

uint8_t getDigit(uint8_t value)
{
  if (value < 10)
  {
    return 1;
  }
  else if (value < 100)
  {
    return 2;
  }
  else
  {
    return 3;
  }
  return 0;
}

void indexMacCal(uint8_t mac[], uint8_t index_input, uint8_t &index_output, String &str_output)
{
  // index_input <= 3 : xx:xx:xx:xx
  // index_input >3 && index_input< 6: xx:xx
  str_output = "";
  for (int i = index_input <= 3 ? 0 : 4; i < 6; i++)
  {
    str_output += mac[i] < 16 ? "0" : "";
    str_output += String(mac[i], HEX);
    if (i < 5)
    {
      str_output += ":";
    }
  }
  // cal index_output
  switch (index_input)
  {
  case 0:
    index_output = 1;
    break;
  case 1:
    index_output = 4;
    break;
  case 2:
    index_output = 7;
    break;
  case 3:
    index_output = 10;
    break;
  case 4:
    index_output = 1;
    break;
  case 5:
    index_output = 4;
    break;
  }
}

// -------------------- END MENU -------------------- //

// -------------------- Manage SD card data -------------------- //
/*
  1. Read data from SD card
  2. Write data to SD card
  3. Append data to SD card
  4. Delete data from SD card
*/

// 10 - 60 convert to A - Z
const char _letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
void generateFileName()
{
  
  int indexName = 0;
  // Clear file
  memset(fileName, 0, FILE_NAME_SIZE);
  // fileName[indexName++] = 'T';
  // Get date
  int n = 0; // xx/xx/xxxx
  // Year last 2 digits
  int digitYear = 0;
  String strD = "";
  int b = 0;
  /*
  for (int i = 0; i < strlen(myDate); i++)
  {
    if (myDate[i] == '/')
    {
      b++;
    }
    else if (b == 2 && digitYear < 2)
    {
      digitYear++;
      continue;
    }

    if (myDate[i] != '/')
    {
      n++;
      strD += myDate[i];
      if (n == 2)
      {
        int num = strD.toInt();
        if (num < 10)
        {
          fileName[indexName++] = char(num + 48);
        }
        else
        {
          fileName[indexName++] = _letters[num - 10];
        }
        strD = "";
        n = 0;
      }
    }
  }
  strD = "";
  // Get time
  for (int i = 0; i < strlen(myTime); i++)
  {
    if (myTime[i] != ':')
    {
      n++;
      strD += myTime[i];
      if (n == 2)
      {
        int num = strD.toInt();
        if (num < 10)
        {
          fileName[indexName++] = char(num + 48);
        }
        else
        {
          fileName[indexName++] = _letters[num - 10];
        }
        strD = "";
        n = 0;
      }
    }
  }
  if (indexName < 5)
  {
    fileName[indexName++] = 'T';
  }

  */
  fileName[indexName++] = 'T';
  String str = String(totalNumberFile);
  // Add to file name
  for (int i = 0; i < str.length(); i++)
  {
    fileName[indexName++] = str[i];
  }
  // Add file extension
  strcat(fileName, ".txt");
  fileName[strlen(fileName)] = '\0'; // terminate the string

  Serial.print("->");
  Serial.print(fileName);
  Serial.println("");
}

String decodeFileName(const char *filename)
{
  String str = "";
  for (int i = 0; i < strlen(filename); i++)
  {
    if (filename[i] >= 48 && filename[i] <= 57)
    {
      str += filename[i];
    }
    else if (filename[i] >= 65 && filename[i] <= 90)
    {
      str += String(filename[i] - 55);
    }
    else if (filename[i] >= 97 && filename[i] <= 122)
    {
      str += String(filename[i] - 61);
    }
  }
  return str;
}

void readSDCard(const char *filename)
{
  // Open file for reading data and BASE_PATH
  File file = SD.open(filename);
  if (file)
  {
    Serial.println("Read from file:");
    while (file.available())
    {
      Serial.write(file.read());
    }
    file.close();
  }
  else
  {
    Serial.println("Failed to open file for reading");
  }
}

void appendFile(const char *filename, const char *message)
{
  // Open file for appending data
  File file = SD.open(filename, FILE_WRITE);
  if (file)
  {
    file.println(message);
    file.close();
  }
  else
  {
    Serial.println("Failed to open file for appending");
  }
}

void lockJigOnEventChange(bool state)
{

  if (statusServer)
  {
    Serial3.println("$PUB=S1_JIG_LOCK:" + String(state ? "LOCK" : "UNLOCK") + "#");
  }
  else
  {
    String message = state ? "JIG=LOCK" : "JIG=UNLOCK";
    appendFile(fileName, message.c_str());
  }
}
void torqueOnEventChange(bool state)
{
  if (statusServer)
  {
    Serial3.println("$PUB=S1_TORQUE:" + String(state ? "ON" : "OFF") + "#");
  }
  else
  {
    String message = "TORQUE=" + String(state ? "ON" : "OFF");
    appendFile(fileName, message.c_str());
  }
}

uint32_t getTotalNumberOfFiles()
{
  uint32_t totalFiles = 0;
  File root = SD.open("/");
  while (true)
  {
    File entry = root.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    if (entry.isDirectory())
    {
      // skip directories
      continue;
    }
    totalFiles++;
    entry.close();
  }
  root.close();
  return totalFiles;
}