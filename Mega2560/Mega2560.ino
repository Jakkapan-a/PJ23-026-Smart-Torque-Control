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

uint8_t passToneCount = 0;
uint8_t totalTonePASS = 2;

uint8_t ngToneCount = 0;
uint8_t totalToneNG = 15;

#define BUZZER_PIN 2
int TONE = 2000;
int DURATION = 200;

// -------------------- Global Variables -------------------- //

// uint32_t previousMillisPASS = 0;
// const long intervalPASS = 200;

// uint32_t previousMillisNG = 0;
// const long intervalNG = 100;

bool stateComplete = false;
bool stateRun = false;
bool stateStop = false;

bool stateEsc = false;
bool stateUp = false;
bool stateDown = false;
bool stateEnter = false;
bool stateButtonReleased = false;

bool currentStateEsc = false;
bool currentStateUp = false;
bool currentStateDown = false;
bool currentStateEnter = false;
bool stateCensorOnStation = false;

const int pressTime = 1500;
int countPressUp = 0;
int countPressDown = 0;

uint32_t timeStart = 0;
uint32_t timeComplete = 0;
uint32_t timeTotal = 0;
uint32_t lastDebounceTime = 0;
uint32_t lastDebounceTimeMillis = 0;
uint8_t indexModel = 0;
uint32_t lastTimeTone = 0;
int addressModel[10] = { 1, 33, 65, 97, 129, 161, 193, 225, 257, 289 };

// const int cal_std_max = 10;
// int cal_std[10] = {0,0,0,0,0,0,0,0,0,0};
// int setCal_std[10] = {0,0,0,0,0,0,0,0,0,0};
uint8_t number_cal = 0;

int indexAddressModel = 0;
String model = "";
String modelSetName = "";
int indexModelName = 0;         // Index edit model name
uint8_t lengthNameModel = 5;    // 5 Digit

String _strCalMin = "";
String _strCalMax = "";

int indexNumber = 0;
uint8_t lengthNumber = 5;       // 5 Digit name

uint8_t countScrew = 0;

int indexMenu = 0;
int selectMenu = 0;
int selectSubMenu = 0;
int selectSubMenu1 = 0;
int selectSubMenu2 = 0;
bool isSave = false;

// Define an array containing letters, numbers, and some symbols
const char letters[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                         'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                         'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                         'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                         '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                         '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_', '=', '+' };


                         
int numChars = sizeof(letters) - 1;
int indexChar = 0;

const char lettersNumber[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
int numCharsNumber = sizeof(lettersNumber) - 1;
int indexCharNumber = 0;

int Min = 0;
int Center = 0;
int Max = 0;

uint16_t stdMin = 0;
uint16_t stdMax= 0;

uint8_t SCW_count = 0;
uint8_t setSCW_count = 0;

uint16_t setCalMin = 0;
uint16_t setCalMax = 0;

void readArray(int indexM, int* data, int dataSize = 10);
int NG_count = 0;
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

  Serial.println("Start...");
  lcd.begin();
  lcd.clear();
  // Turn on the blacklight and print a message.
  lcd.backlight();

  indexMenu = 0;
  indexModel = readInt8InEEPROM(0);
  UpdateCountControlsSCW();
  model = readEEPROM(addressModel[indexModel], lengthNameModel);
  
  UpdateCalSTD(getMin(addressModel[indexModel]),getMax(addressModel[indexModel]));
}

void loop() {
  btnRun.update();
  btnStop.update();
  btnEsc.update();
  btnUp.update();
  btnDown.update();
  btnEnter.update();
  btnCensorOnSt.update();
  DisplayMenu();

   if(stateCensorOnStation && ngToneCount > 1){
    ngToneCount = totalToneNG;
  }

  ToneFun(currentMillis, 200, 2000, 50, passToneCount); //
  ToneFun(currentMillis, 100, 2000, 50, ngToneCount);

  
  // handlePassTone();
  // handleNGTone();

  if (!stateRun && !stateStop && stateComplete) {
    stateComplete = false;
  }
  if (stateRun && stateStop && !stateComplete && stateCensorOnStation) {
    Serial.println("Completed: ");

    stateComplete = true;
    timeComplete = millis();
    timeTotal = timeComplete - timeStart;
    countScrew++;

    String dataFinish = "$";
    dataFinish += "SCW:";
    dataFinish += String(countScrew);
    dataFinish+= ",";
    dataFinish += "FINISH_TIME:";
    dataFinish += String(timeTotal);
    // RESULT
    dataFinish += ",";

    if(timeTotal < static_cast<uint32_t>(CalStdMin) || timeTotal > static_cast<uint32_t>(CalStdMax)){
      LED_Controls(1);
      status_test = NG;
      ngToneCount = totalToneNG;
      NG_count++;
      // countScrew--;
      dataFinish +="RESULT:NG";
    }else    
    if(countScrew > SCW_count){
      LED_Controls(1);
      status_test = NG;
      ngToneCount = totalToneNG;
      dataFinish +="RESULT:NG";
    }else
    {
      LED_Controls(0);
      tone(BUZZER_PIN, 2000, 200);
      dataFinish +="RESULT:PASS";
    }
    dataFinish += "#";
    Serial.println(dataFinish);
    // Update LCD
    // updateLCD("Completed: ", String(timeTotal) + " ms");
    // if (selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 2){
    //     setCal_std[selectSubMenu2-1] = timeTotal;
    //     selectSubMenu2++;
    //     if(selectSubMenu2>10){
    //       selectSubMenu2 = 1;
    //       // Enter to Save
    //       EnterSetCalStd();
    //     }
    //   }
    // Clear
    // timeTotal = 0;

  }
}
void LED_Controls(uint8_t state = 0){
  if(state == 0){
    ledRed.off();
    ledGreen.off();
    ledBlue.off();
  }else if(state == 1){
    ledRed.on();
    ledGreen.off();
    ledBlue.off();
  }else  if(state == 2){
    ledRed.off();
    ledGreen.on();
    ledBlue.off();
  }else  if(state == 3){
    ledRed.off();
    ledGreen.off();
    ledBlue.on();
  }
}

void UpdateCalSTD(uint16_t _min, uint16_t _max)
{
  CalStdMin = _min;// Min-Center;
  CalStdMax = _max; // Max+Center;
}
bool isRun =false;
void DisplayMenu() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > 10) {
   if (indexMenu == 1) {
      SettingMenu();
    }
    lastDebounceTime = currentMillis;
  } else if (currentMillis < lastDebounceTime) {  // Overflows
    lastDebounceTime = currentMillis;
  }

  if (currentMillis - lastDebounceTimeMillis > 100) {
  //  if (stateRun && !stateStop && !stateComplete && stateCensorOnStation) {
  //     String dataRun = "$";
  //     dataRun += "SCW:";
  //     dataRun += String(countScrew);
  //     dataRun+= ",";
  //     dataRun += "RUN:";
  //     dataRun += String(millis() - timeStart);
  //     dataRun+= "#";
  //     Serial.println(dataRun);
  //     isRun = true;
  //   }else if(isRun &&!stateRun && !stateStop && !stateComplete && stateCensorOnStation){
  //     isRun = false;
  //     String dataRun = "$";
  //     dataRun += "STOP:0";
  //     dataRun+= "#";
  //     Serial.println(dataRun);
  //   }

    if (indexMenu == 0) {
      HomeDisplay();
    }

    stateButtonPressed();
    resetStateButton();
    if (currentStateUp) {
      if (countPressUp > pressTime) {
        btnUpOnEventPressed();
      } else {
        countPressUp += 100;
      }
    } else {
      countPressUp = 0;
    }

    if (currentStateDown) {
      if (countPressDown > pressTime) {
        btnDownOnEventPressed();
      } else {
        countPressDown += 100;
      }
    } else {
      countPressDown = 0;
    }
    lastDebounceTimeMillis = currentMillis;
  } else if (currentMillis < lastDebounceTimeMillis) {
    lastDebounceTimeMillis = currentMillis;
  }

 
}

void ToneFun(uint32_t _currentMillis, uint32_t _toneTime, int _toneFreq, uint8_t _tonePercent, uint8_t &totalTone) {
  if (totalTone <= 0) {
    return;
  }
  // uint32_t currentMillis = millis();
  if (_currentMillis - lastTimeTone > _toneTime) {
    if (_tonePercent > 0) {
      int p = (int)(_tonePercent * _toneTime / 100);
      tone(BUZZER_PIN, _toneFreq, p);
      totalTone--;
    }
    lastTimeTone = _currentMillis;
  } else if (_currentMillis < lastTimeTone) {
    lastTimeTone = _currentMillis;
  }
}


void HomeDisplay() {
  String line1 = "MODEL " + model;
  String line2 = String(countScrew) + "/"+String(SCW_count)+"PCS," + String(timeTotal) + "ms";
  // 1 Millisecond
  if (stateRun && !stateStop && !stateComplete) {
    line2 = String(countScrew) + "/"+String(SCW_count)+"PCS," + String(millis() - timeStart) + "ms";
  }

  if(!stateCensorOnStation){
    line2 = "----------------";
  }
  updateLCD(line1, line2);
}

String getModelName(int index) {
  if (index < 0 || index > 9) {
    return "";
  }
  return readEEPROM(addressModel[index], lengthNameModel);
}

void SettingMenu() {

  String line1 = "               ";
  String line2 = "               ";
  if (isSave && selectSubMenu > 0) {
    line1 = "SAVE...........";
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
  if (selectMenu == 0 && selectSubMenu > 0) {
    if (selectSubMenu == 1) {
      String model1 = getModelName(0);
      String model2 = getModelName(1);
      line1 = ">1.MODEL " + model1;
      line2 = " 2.MODEL " + model2;
    } else if (selectSubMenu == 2) {
      String model1 = getModelName(0);
      String model2 = getModelName(1);
      line1 = " 1.MODEL " + model1;
      line2 = ">2.MODEL " + model2;
    } else if (selectSubMenu == 3) {
      String model1 = getModelName(2);
      String model2 = getModelName(3);
      line1 = ">3.MODEL " + model1;
      line2 = " 4.MODEL " + model2;
    } else if (selectSubMenu == 4) {
      String model1 = getModelName(2);
      String model2 = getModelName(3);
      line1 = " 3.MODEL " + model1;
      line2 = ">4.MODEL " + model2;
    } else if (selectSubMenu == 5) {
      String model1 = getModelName(4);
      String model2 = getModelName(5);
      line1 = ">5.MODEL " + model1;
      line2 = " 6.MODEL " + model2;
    } else if (selectSubMenu == 6) {
      String model1 = getModelName(4);
      String model2 = getModelName(5);
      line1 = " 5.MODEL " + model1;
      line2 = ">6.MODEL " + model2;
    } else if (selectSubMenu == 7) {
      String model1 = getModelName(6);
      String model2 = getModelName(7);
      line1 = ">7.MODEL " + model1;
      line2 = " 8.MODEL " + model2;
    } else if (selectSubMenu == 8) {
      String model1 = getModelName(6);
      String model2 = getModelName(7);
      line1 = " 7.MODEL " + model1;
      line2 = ">8.MODEL " + model2;
    } else if (selectSubMenu == 9) {
      String model1 = getModelName(8);
      String model2 = getModelName(9);
      line1 = ">9.MODEL " + model1;
      line2 = " 10.MODEL " + model2;
    } else if (selectSubMenu == 10) {
      String model1 = getModelName(8);
      String model2 = getModelName(9);
      line1 = " 9.MODEL " + model1;
      line2 = ">10.MODEL " + model2;
    } else if (selectSubMenu > 10) {
      selectSubMenu = 1;
    } else if (selectSubMenu < 1) {
      selectSubMenu = 10;
    }

    indexAddressModel = selectSubMenu - 1;
  }

  // --------------------------------- Setting model ------------------------- //
  else if (selectMenu == 1 && selectSubMenu > 0) {

    if (selectSubMenu1 > 0) {
      // ---------------------  Parameter By Model ------------------ //
        // ---------------------  Name ------------------ //
      if (selectSubMenu1 == 1) {
        line1 = ">NAME";
        line2 = " COUNT CONTROLS";
        if (selectSubMenu2 > 0) {
          lcd.setCursor(indexModelName, 1);
          lcd.cursor();
          lcd.blink();
          char buf[lengthNameModel + 1];
          for (int i = 0; i < lengthNameModel + 1; i++) {
            buf[i] = ' ';
          }
          modelSetName.toCharArray(buf, lengthNameModel + 1);
          buf[indexModelName] = letters[indexChar];
          modelSetName = String(buf);
          line1 = "MODEL NAME:";
          line2 = modelSetName;
        }
        // --------------------- end Name ------------------ //
      } else if (selectSubMenu1 == 2) {
        line1 = " NAME";
        line2 = ">COUNT CONTROLS";
        if (selectSubMenu2 > 0) {
          if(selectSubMenu2>0){
            line1 ="SCW COUNT:      "; 
            line2 = String(setSCW_count)+" PCS";
          }
        }

        // --------------------- end Name ------------------ //
      } else if (selectSubMenu1 == 3) {
        line1 = ">MIN:"+String(setCalMin);
        line2 = " MAX:"+String(setCalMax);
        if (selectSubMenu2 > 0) {

          // indexNumber
          lcd.setCursor(indexNumber, 1);
          lcd.cursor();
          lcd.blink();

          char buf[lengthNumber + 1];
          for (int i = 0; i < lengthNumber + 1; i++) {
            buf[i] = ' ';
          }
          
          _strCalMin = ConvertNumberToString(setCalMin);

          _strCalMin.toCharArray(buf, lengthNumber + 1);
          
          buf[indexNumber] = lettersNumber[indexCharNumber];

          // unsigned int
          String bufString = String(buf); // Create a String object from the char array
          setCalMin = ConvertStringToNumber(bufString);//bufString.toInt(); // Convert the String to an integer
          
          line1 = "SET MIN:      ";
          line2 = ConvertNumberToString(setCalMin) +" ms";
        }
      }
       else if (selectSubMenu1 == 4) {
        line1 = " MIN:"+String(setCalMin);
        line2 = ">MAX:"+String(setCalMax);
        if (selectSubMenu2 > 0) {

          // indexNumber
          lcd.setCursor(indexNumber, 1);
          lcd.cursor();
          lcd.blink();

          char buf[lengthNumber + 1];
          for (int i = 0; i < lengthNumber + 1; i++) {
            buf[i] = ' ';
          }
          
          _strCalMax = ConvertNumberToString(setCalMax);

          _strCalMax.toCharArray(buf, lengthNumber + 1);
          
          buf[indexNumber] = lettersNumber[indexCharNumber];

          // unsigned int
          String bufString = String(buf); // Create a String object from the char array
          setCalMax = ConvertStringToNumber(bufString);//bufString.toInt(); // Convert the String to an integer
          
          line1 = "SET MAX:      ";
          line2 = ConvertNumberToString(setCalMax) +" ms";
          
        }
      }
      // ---------------------  End Parameter By Model ------------------ //
      else if (selectSubMenu1 > 4) {
        selectSubMenu1 = 1;
      } else if (selectSubMenu1 < 1) {
        selectSubMenu1 = 5;
      }
      // ---------------------  End Parameter By Model ------------------ //
    } else if (selectSubMenu == 1) {
      String model1 = getModelName(0);
      String model2 = getModelName(1);
      line1 = ">1.MODEL " + model1;
      line2 = " 2.MODEL " + model2;
    } else if (selectSubMenu == 2) {
      String model1 = getModelName(0);
      String model2 = getModelName(1);
      line1 = " 1.MODEL " + model1;
      line2 = ">2.MODEL " + model2;
    } else if (selectSubMenu == 3) {
      String model1 = getModelName(2);
      String model2 = getModelName(3);
      line1 = ">3.MODEL " + model1;
      line2 = " 4.MODEL " + model2;
    } else if (selectSubMenu == 4) {
      String model1 = getModelName(2);
      String model2 = getModelName(3);
      line1 = " 3.MODEL " + model1;
      line2 = ">4.MODEL " + model2;
    } else if (selectSubMenu == 5) {
      String model1 = getModelName(4);
      String model2 = getModelName(5);
      line1 = ">5.MODEL " + model1;
      line2 = " 6.MODEL " + model2;
    } else if (selectSubMenu == 6) {
      String model1 = getModelName(4);
      String model2 = getModelName(5);
      line1 = " 5.MODEL " + model1;
      line2 = ">6.MODEL " + model2;
    } else if (selectSubMenu == 7) {
      String model1 = getModelName(6);
      String model2 = getModelName(7);
      line1 = ">7.MODEL " + model1;
      line2 = " 8.MODEL " + model2;
    } else if (selectSubMenu == 8) {
      String model1 = getModelName(6);
      String model2 = getModelName(7);
      line1 = " 7.MODEL " + model1;
      line2 = ">8.MODEL " + model2;
    } else if (selectSubMenu == 9) {
      String model1 = getModelName(8);
      String model2 = getModelName(9);
      line1 = ">9.MODEL " + model1;
      line2 = " 10.MODEL " + model2;
    } else if (selectSubMenu == 10) {
      String model1 = getModelName(8);
      String model2 = getModelName(9);
      line1 = " 9.MODEL " + model1;
      line2 = ">10.MODEL " + model2;
    } else if (selectSubMenu > 10) {
      selectSubMenu = 1;
    } else if (selectSubMenu < 1) {
      selectSubMenu = 10;
    }
    indexAddressModel = selectSubMenu - 1;
  }

  // --------------------------------- Main Menu ------------------------------//
  else if (selectMenu == 0) {
    line1 = ">SELECT MODEL";
    line2 = " SETTING MODEL";
  } else if (selectMenu == 1) {
    line1 = " SELECT MODEL";
    line2 = ">SETTING MODEL";
  } else if (selectMenu == 2) {
    line1 = ">SYSTEM";
  } else if (selectMenu > 2) {
    selectMenu = 0;
  } else if (selectMenu < 0) {
    selectMenu = 2;
  }
  updateLCD(line1, line2);
}

String ConvertNumberToString(uint16_t number) {
  String str = "";
  if (number < 10) {
    str = "0000" + String(number);
  } else if (number < 100) {
    str = "000" + String(number);
  } else if (number < 1000) {
    str = "00" + String(number);
  } else if (number < 10000) {
    str = "0" + String(number);
  } else {
    str = String(number);
  }
  return str;
}

uint16_t ConvertStringToNumber(String str) {
  return (uint16_t)str.toInt();
}

void btnRunOnEventChange(bool state) {
  stateRun = !state;
  if (stateRun) {
    timeStart = millis();
    LED_Controls(3);
    // ngToneCount = 5;
  }else{
    // int _timeTotal = (int)timeTotal;
    if(timeTotal < static_cast<uint32_t>(stdMin) || timeTotal > static_cast<uint32_t>(stdMax) ){
      LED_Controls(1);
      // status_test = NG;
    }else{
      LED_Controls(0);
    }
  }
}

void btnStopOnEventChange(bool state) {
  stateStop = !state;
}
void btnCensorOnStOnEventChange(bool state){
  stateCensorOnStation = !state;
  if(!stateCensorOnStation){
    // 
    if(countScrew > 0 && countScrew == SCW_count && NG_count == 0){
      status_test = PASS;
      Serial.println("$JUDGEMENT:PASS#");
      LED_Controls(2);
      passToneCount = totalTonePASS;
    }else if(countScrew >0){
      LED_Controls(1);
      Serial.println("$JUDGEMENT:NG#");
      status_test = NG;
      ngToneCount = totalToneNG;
    }
    countScrew = 0;
    NG_count = 0;

  }else{
    // LED_Controls(3);
    
    Serial.println("$RESET:1#");
    ngToneCount = 1;
  }
}

String currentLine1 = "                ";  //  16 ตัว
String currentLine2 = "                ";  //  16 ตัว

void updateLCD(const String &newDataLine1, const String &newDataLine2) {
  // Line 1
  int len = newDataLine1.length();
  for (int i = 0; i < 16; i++) {
    if (i < len) {
      if (newDataLine1[i] != currentLine1[i]) {
        lcd.setCursor(i, 0);
        lcd.print(newDataLine1[i]);
        currentLine1[i] = newDataLine1[i];
      }
    } else {
      if (currentLine1[i] != ' ') {
        lcd.setCursor(i, 0);
        lcd.print(' ');
        currentLine1[i] = ' ';
      }
    }
  }

  // Line 2
  len = newDataLine2.length();
  for (int i = 0; i < 16; i++) {
    if (i < len) {
      if (newDataLine2[i] != currentLine2[i]) {
        lcd.setCursor(i, 1);
        lcd.print(newDataLine2[i]);
        currentLine2[i] = newDataLine2[i];
      }
    } else {
      if (currentLine2[i] != ' ') {
        lcd.setCursor(i, 1);
        lcd.print(' ');
        currentLine2[i] = ' ';
      }
    }
  }
}

void btnEscOnEventChange(bool state) {
  currentStateEsc = !state;
  if (!state) {
    stateEsc = true;
    Serial.println("ESC");
    tone(BUZZER_PIN, 2400, 100);
    // btnEscOnEventPressed();
    lastDebounceTimeMillis = millis();
  } else {
    stateButtonReleased = true;
  }
}
void btnEscOnEventPressed() {
  if (selectSubMenu2 > 0) {
    selectSubMenu2 = 0;
  } else if (selectSubMenu1 > 0) {
    selectSubMenu1 = 0;
  } else if (selectSubMenu > 0) {
    selectSubMenu = 0;
  } else if (selectMenu > 0) {
    selectMenu = 0;
  } else if (indexMenu == 0) {
    indexMenu = 1;
  } else if (indexMenu == 1) {
    indexMenu = 0;
  }
  lcd.noBlink();
  lcd.noCursor();
}
void btnUpOnEventChange(bool state) {
  currentStateUp = !state;
  if (!state) {
    stateUp = true;
    Serial.println("UP");
    tone(BUZZER_PIN, 2400, 100);
    // btnUpOnEventPressed();
    lastDebounceTimeMillis = millis();
  } else {
    stateButtonReleased = true;
  }
}

void btnUpOnEventPressed() {
  if (selectSubMenu2 > 0) {
    // MIN MAX
     if (selectMenu == 1 && (selectSubMenu1 == 3 || selectSubMenu1 == 4)){
      indexCharNumber++;
      if (indexCharNumber > numCharsNumber) {
        indexCharNumber = 0;
      }

      if(indexNumber == 0 && indexCharNumber > 5){
        indexCharNumber = 0;
      }

     }else
    // SCW_count
     if (selectMenu == 1 && selectSubMenu1 == 2){
      // selectSubMenu2++;
      // if(selectSubMenu2>10){
      //   selectSubMenu2 = 1;
      // }
       setSCW_count++;
        if(setSCW_count>50){
          setSCW_count = 1;
        }

     }else
     // Model Name
    if (selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1) {
      indexChar++;
      if (indexChar > numChars) {
        indexChar = 0;
      }
    }
  } else if (selectSubMenu1 > 0) {
    selectSubMenu1--;
    if (selectSubMenu1 < 1) {
      selectSubMenu1 = 5;
    }
  } else if (selectSubMenu > 0) {
    selectSubMenu--;
    if (selectMenu == 0 || selectMenu == 1) {
      if (selectSubMenu < 1) {
        selectSubMenu = 10;
      }
    }
  } else {
    selectMenu--;
    if (selectMenu < 0) {
      selectMenu = 2;
    }
  }
}
void btnDownOnEventChange(bool state) {
  currentStateDown = !state;
  if (!state) {
    stateDown = true;
    Serial.println("DOWN");
    tone(BUZZER_PIN, 2400, 100);
    // btnDownOnEventPressed();
    lastDebounceTimeMillis = millis();
  } else {
    stateButtonReleased = true;
  }
}

void btnDownOnEventPressed() {
  if (selectSubMenu2 > 0) {
    // Count
     if (selectMenu == 1 && (selectSubMenu1 == 3 || selectSubMenu1 == 4)){
      indexCharNumber--;
      if (indexCharNumber < 0) {
        indexCharNumber = numCharsNumber;
      }

      if(indexNumber == 0 && indexCharNumber > 5){
        indexCharNumber = 5;
      }

     }else
     if (selectMenu == 1 && selectSubMenu1 == 2){
       setSCW_count--;
        if(setSCW_count<1){
          setSCW_count = 50;
        }
     }else
    if (selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1) {
      indexChar--;
      if (indexChar < 0) {
        indexChar = numChars;
      }
    }
  } else if (selectSubMenu1 > 0) {
    selectSubMenu1++;
  } else if (selectSubMenu > 0) {
    selectSubMenu++;
  } else {
    selectMenu++;
  }
}

void btnUpDownOnEventPressed() {
  Serial.println("UP DOWN");
  if (selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1) {
    indexModelName = 0;
    resetIndexChar();
  }else 
  if(selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 3){
    indexNumber = 0;
    resetIndexCharNumber(ConvertNumberToString(setCalMin));
  }if(selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 4){
    indexNumber = 0;
    resetIndexCharNumber(ConvertNumberToString(setCalMax));
  }
}
void btnEnterOnEventChange(bool state) {
  currentStateEnter = !state;
  if (!state) {
    stateEnter = true;
    Serial.println("ENTER");
    tone(BUZZER_PIN, 2400, 100);
    // btnEnterOnEventPressed();
    lastDebounceTimeMillis = millis();
  } else {
    stateButtonReleased = true;
  }
}

void btnEnterOnEventPressed() {
  if (indexMenu == 0) {
    return;
  }
  if (selectMenu == 0 && selectSubMenu == 0) {
    selectSubMenu = 1;
  } else if (selectMenu == 0 && selectSubMenu > 0) {
    isSave = true;
    // Select model
    int index = selectSubMenu - 1;
    updateEEPROM(0, index);
    model = readEEPROM(addressModel[index], lengthNameModel);
  } else if (selectMenu == 1 && selectSubMenu == 0) {
    selectSubMenu = 1;
  } else 
  
  if (selectMenu == 1 && selectSubMenu > 0 && selectSubMenu1 == 0) {
      selectSubMenu1 = 1;
      setCalMin = getMin(addressModel[indexAddressModel]);
      setCalMax = getMax(addressModel[indexAddressModel]);
  } else if (selectMenu == 1 && selectSubMenu > 0 && selectSubMenu1 > 0) {
    EnterSetName();
    // EnterSetCalStd();
    EnterSetCountControl();
    EnterMin();
    EnterMax();
  }
}

void EnterSetName()
{
  if(selectSubMenu1 != 1) return;
  // 
  if (selectSubMenu2 == 0) {
    selectSubMenu2 = 1;
    modelSetName = readEEPROM(addressModel[indexAddressModel], lengthNameModel);
    // modelSetName = model;
    indexModelName = 0;
    resetIndexChar();
  } else if (selectSubMenu2 > 0) {
    if (selectMenu == 1 && selectSubMenu1 == 1 && selectSubMenu2 == 1) {
      indexModelName++;
      resetIndexChar();
      if (indexModelName >= lengthNameModel) {
        indexModelName = 0;

        updateLCD("Save...........", "               ");
        // model = modelSetName;
        // Save to EEPROM
        updateEEPROM(addressModel[indexAddressModel], modelSetName);
        // Read EEPROM
        if(indexAddressModel == indexModel){
          model = readEEPROM(addressModel[indexAddressModel], lengthNameModel);
        }
        delay(1000);
        // Clear lcd
        lcd.noBlink();
        lcd.noCursor();
        // Update LCD
        updateLCD("Save Completed ", "               ");
        resetIndexChar();
        selectSubMenu2 = 0;
      }
    }
  }
}

void EnterSetCalStd()
{
    if(selectSubMenu1 != 2) return;
    if(selectSubMenu2 == 0) 
    {
      // Set 
      selectSubMenu2 = 1;
    } else if (selectSubMenu2 > 0) {
       updateLCD("Save...........", "               ");
      // Save to EEPROM
      // int address = getAddress(addressModel[indexAddressModel], selectSubMenu2-1);
    
      // for(int i = 0; i<10; i++)
      // {
      //    int _data = setCal_std[i];
      //    int address = getAddress(addressModel[indexAddressModel], i);
      //    // Update to EEPROM
      //    updateInt16ToEEPROM(address,_data);
      //    // 
      //    if(indexAddressModel == indexModel){
      //      UpdateCalSTD();
      //    }
      // }
       delay(1000);
       selectSubMenu2 = 0;
    }
}

void EnterSetCountControl(){
  if(selectSubMenu1 != 2) return;

  if(selectSubMenu2 == 0) 
  {
    setSCW_count = getCountControls(addressModel[indexAddressModel]);
    // Set 
    selectSubMenu2 = 1;
  }else if (selectSubMenu2 > 0) {
      updateLCD("Save...........", "               ");
      // Save to EEPROM
      uint8_t address = getAddress(addressModel[indexAddressModel],10);
      updateEEPROM(address, setSCW_count);
      if(indexAddressModel == indexModel){
        UpdateCountControlsSCW();
      }
      delay(1000);
      selectSubMenu2 = 0;
  }
}

void EnterMin(){
  if(selectSubMenu1 != 3) return;
  if(selectSubMenu2 == 0) 
  {
    selectSubMenu2 = 1;
    // Read EEPROM
    setCalMin = getMin(addressModel[indexAddressModel]);
        
    indexNumber = 0;
    resetIndexCharNumber(ConvertNumberToString(setCalMin));

  }else if (selectSubMenu2 > 0) {
      indexNumber++;
      resetIndexCharNumber(ConvertNumberToString(setCalMin));
      if (indexNumber >= lengthNumber) {
        indexNumber = 0;
        updateLCD("Save...........", "               ");
        // Save to EEPROM
        uint8_t address = getAddress(addressModel[indexAddressModel],0);
        updateInt16ToEEPROM(address, setCalMin);
        if(indexAddressModel == indexModel){
            CalStdMin = setCalMin;
        }
        delay(1000);
        selectSubMenu2 = 0;
      }
  }
}

void EnterMax(){
  if(selectSubMenu1 != 4) return;
  if(selectSubMenu2 == 0) 
  {
    setCalMax = getMax(addressModel[indexAddressModel]);
    // Set 
    selectSubMenu2 = 1;

    indexNumber = 0;
    resetIndexCharNumber(ConvertNumberToString(setCalMax));
  }else if (selectSubMenu2 > 0) {
      indexNumber++;
      resetIndexCharNumber(ConvertNumberToString(setCalMax));
      if (indexNumber >= lengthNumber) {
        indexNumber = 0;
        updateLCD("Save...........", "               ");
        // Save to EEPROM
        uint8_t address = getAddress(addressModel[indexAddressModel],1);
        updateInt16ToEEPROM(address, setCalMax);
        if(indexAddressModel == indexModel){
            CalStdMax = setCalMax;
        }
        delay(1000);
        selectSubMenu2 = 0;
      }
  }
}

void UpdateCountControlsSCW(){
  SCW_count = getCountControls(addressModel[indexModel]);
}

void resetIndexChar() {
  for (int i = 0; i < numChars; i++) {
    if (letters[i] == modelSetName[indexModelName]) {
      indexChar = i;
      break;
    }
  }
}

void resetIndexCharNumber(String _cal) {
  //String _calMax = ConvertNumberToString(setCalMax);
  for (int i = 0; i < numCharsNumber; i++) {
    if (lettersNumber[i] == _cal[indexNumber]) {
      indexCharNumber = i;
      break;
    }
  }
}


void stateButtonPressed() {
  // 0 0 0 0
  if (!stateEsc && !stateUp && !stateDown && !stateEnter) {
    // not pressed
    return;
  } else
    // 0 0 0 1
    if (!stateEsc && !stateUp && !stateDown && stateEnter) {
      btnEnterOnEventPressed();
    } else
      // 0 0 1 0
      if (!stateEsc && !stateUp && stateDown && !stateEnter) {
        btnDownOnEventPressed();
      } else
        // 0 0 1 1
        if (!stateEsc && !stateUp && stateDown && stateEnter) {
          // btnDownOnEventPressed();
          // btnEnterOnEventPressed();
        } else
          // 0 1 0 0
          if (!stateEsc && stateUp && !stateDown && !stateEnter) {
            btnUpOnEventPressed();
          } else
            // 0 1 0 1
            if (!stateEsc && stateUp && !stateDown && stateEnter) {
              // btnUpOnEventPressed();
              // btnEnterOnEventPressed();
            } else
              // 0 1 1 0
              if (!stateEsc && stateUp && stateDown && !stateEnter) {
                // btnUpOnEventPressed();
                btnUpDownOnEventPressed();
                // Serial.println("UP DOWN");
              } else
                // 0 1 1 1
                if (!stateEsc && stateUp && stateDown && stateEnter) {
                  // btnUpOnEventPressed();
                  // btnDownOnEventPressed();
                  // btnEnterOnEventPressed();
                } else
                  // 1 0 0 0
                  if (stateEsc && !stateUp && !stateDown && !stateEnter) {
                    btnEscOnEventPressed();
                  }

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
}

void resetStateButton() {
  stateEsc = false;
  stateUp = false;
  stateDown = false;
  stateEnter = false;
}
void updateEEPROM(int index, uint8_t data) {
  EEPROM.update(index, data);
}
void updateInt16ToEEPROM(int address, uint16_t data) {
  EEPROM.update(address, data >> 8); // Store the higher byte
  EEPROM.update(address + 1, data & 0xFF); // Store the lower byte
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
  data = EEPROM.read(index) << 8; // Read the higher byte and shift it
  data |= EEPROM.read(index + 1); // Read the lower byte and OR it
  return data;
}

// void readArray(int indexM, int* data, int dataSize) {
//   int address = indexM + 10;
//   for (int i = 0;i < dataSize; i++) {
//       data[i] = readInt16CInEEPROM(address);; // 
//       address+=2; // Update address
//   }
// }


int getAddress(int indexM, int index)
{
  int address = (indexM + 10) + (index * 2);
  return address;
}
String readEEPROM(int index, int len){
  String data = "";
  for (int i = 0; i < len; i++) {
    data += char(EEPROM.read(index + i));
  }
  return data;
}

uint8_t getCountControls(int address){
  int add = getAddress(address,10);
  return readInt8InEEPROM(add);
}

uint16_t getMin(int address){
  int add = getAddress(address,0);
  return readInt16CInEEPROM(add);
}

uint16_t getMax(int address){
  int add = getAddress(address,1);
  return readInt16CInEEPROM(add);
}