#include <TcBUTTON.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// -------------------- LCD -------------------- //
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- INPUT -------------------- //
#define BTN_RUN 11
void btnRunOnEventChange(bool state);
TcBUTTON btnRun(BTN_RUN);
#define BTN_STOP 12
void btnStopOnEventChange(bool state);
TcBUTTON btnStop(BTN_STOP);

#define BTN_ESC_PIN 35
void btnEscOnEventChange(bool state);
TcBUTTON btnEsc(BTN_ESC_PIN);

#define BTN_UP_PIN 36
void btnUpOnEventChange(bool state);
TcBUTTON btnUp(BTN_UP_PIN);

#define BTN_DOWN_PIN 37
void btnDownOnEventChange(bool state);
TcBUTTON btnDown(BTN_DOWN_PIN);

#define BTN_ENTER_PIN 38
void btnEnterOnEventChange(bool state);
TcBUTTON btnEnter(BTN_ENTER_PIN);

#define BUZZER_PIN 2
int TONE = 2000;
int DURATION = 200;

// -------------------- Global Variables -------------------- //
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
const int pressTime = 1500;
int countPressUp = 0;
int countPressDown = 0;

unsigned long timeStart = 0;
unsigned long timeComplete = 0;
unsigned long timeTotal = 0;
unsigned long lastDebounceTime = 0;
unsigned long lastDebounceTimeMillis = 0;
uint8_t indexModel = 0;

int addressModel[10] = { 1, 33, 65, 97, 129, 161, 193, 225, 257, 289 };

// const int cal_std_max = 10;
int cal_std[10] = {0,0,0,0,0,0,0,0,0,0};
int setCal_std[10] = {0,0,0,0,0,0,0,0,0,0};
uint8_t number_cal = 0;

int indexAddressModel = 0;
String model = "";
String modelSetName = "";
int indexModelName = 0; // Index edit model name
uint8_t lengthNameModel = 8;  // 8 Digit
int countScrew = 0;

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

int Min = 0;
int Center = 0;
int Max = 0;

int CalStdMin = 0;
int CalStdMax= 0;

uint8_t SCW_count = 0;
uint8_t setSCW_count = 0;

int setCalMin = 0;
int setCalMax = 0;
void readArray(int indexM, int* data, int dataSize = 10);

void setup() {
  Serial.begin(115200);
  btnRun.setOnEventChange(btnRunOnEventChange);
  btnRun.setDebounceDelay(20);
  btnStop.setOnEventChange(btnStopOnEventChange);
  btnStop.setDebounceDelay(20);

  btnEsc.setOnEventChange(btnEscOnEventChange);
  btnUp.setOnEventChange(btnUpOnEventChange);
  btnDown.setOnEventChange(btnDownOnEventChange);
  btnEnter.setOnEventChange(btnEnterOnEventChange);

  Serial.println("Start...");
  lcd.begin();
  lcd.clear();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  // lcd.setCursor(0, 0);
  // lcd.print("READY...");

  indexMenu = 0;
  indexModel = readInt8InEEPROM(0);
  UpdateCalSTD();
  UpdateCountControlsSCW();
  model = readEEPROM(addressModel[indexModel], lengthNameModel);

}
void loop() {
  btnRun.update();
  btnStop.update();
  btnEsc.update();
  btnUp.update();
  btnDown.update();
  btnEnter.update();
  DisplayMenu();

  if (!stateRun && !stateStop && stateComplete) {
    stateComplete = false;
  }
  if (stateRun && stateStop && !stateComplete) {
    Serial.println("Completed: ");
    tone(BUZZER_PIN, 2000, 200);
    stateComplete = true;
    timeComplete = millis();
    timeTotal = timeComplete - timeStart;
    Serial.print("Start: ");
    Serial.print(timeStart);
    Serial.println(" ms");
    Serial.print("Complete: ");
    Serial.print(timeComplete);
    Serial.println(" ms");
    Serial.print("Total: ");
    Serial.print(timeTotal);
    Serial.println(" ms");

    // Update LCD
    // updateLCD("Completed: ", String(timeTotal) + " ms");
    if (selectSubMenu2 > 0 && selectMenu == 1 && selectSubMenu1 == 2){
        setCal_std[selectSubMenu2-1] = timeTotal;
        selectSubMenu2++;
        if(selectSubMenu2>10){
          selectSubMenu2 = 1;
          // Enter to Save
          EnterSetCalStd();
        }
      }
    // Clear
    // timeTotal = 0;
    countScrew++;
  }
}

void UpdateCalSTD(){
  readArray(addressModel[indexModel],cal_std);
  Min = 20000;
  Max = 0;
  // Iterate through the array to find the min and max
  for (int i = 0; i < 10; i++) {
    if (cal_std[i] < Min) {
      Min = cal_std[i]; // Update min
    }
    if (cal_std[i] > Max) {
      Max = cal_std[i]; // Update max
    }
    // Serial.print("cal_std ");
    // Serial.println(cal_std[i]);
  }
    Serial.println("====================================");
  // Calculate the center
  Center = (Max - Min) / 2;
  CalStdMin = Max-Center;
  CalStdMax = Max+Center;


  Serial.print("Min: ");
  Serial.println(Min);
  Serial.print("Max: ");
  Serial.println(Max);
  Serial.print("Center: ");
  Serial.println(Center);
  Serial.print("CalStdMin: ");
  Serial.println(CalStdMin);
  Serial.print("CalStdMax: ");
  Serial.println(CalStdMax);
}
void DisplayMenu() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > 10) {
    if (indexMenu == 0) {
      HomeDisplay();
    } else if (indexMenu == 1) {
      SettingMenu();
    }
    lastDebounceTime = currentMillis;
  } else if (currentMillis < lastDebounceTime) {  // Overflows
    lastDebounceTime = currentMillis;
  }

  if (currentMillis - lastDebounceTimeMillis > 100) {
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

void HomeDisplay() {
  String line1 = "MODEL " + model;
  String line2 = String(countScrew) + "/"+String(SCW_count)+"PCS," + String(timeTotal) + "ms";
  // 1 Millisecond
  if (stateRun && !stateStop && !stateComplete) {
    line2 = String(countScrew) + "/"+String(SCW_count)+"PCS," + String(millis() - timeStart) + "ms";
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
        line2 = " CAL STD";
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
        line2 = ">CAL STD";
        if (selectSubMenu2 > 0) {
            line1 = "CAL STD: "+String(selectSubMenu2)+"/10";
            line2 = "              ";
            // 
             if (stateRun && !stateStop && !stateComplete) {
              line2 = "TIME: "+ String(millis() - timeStart)+"ms";
            }else{
              line2 = "TIME: "+ String(setCal_std[selectSubMenu2-1])+"ms";
            }            
          }
        // --------------------- end Name ------------------ //

      } else if (selectSubMenu1 == 3) {
        line1 = ">COUNT CONTROLS";
        line2 = " CAL MIN:"+String(setCalMin);
        if(selectSubMenu2>0){
          line1 ="SCW COUNT:      "; 
          line2 = String(setSCW_count)+" PCS";
    
        }
      }
       else if (selectSubMenu1 == 4) {
        line1 = " COUNT CONTROLS";
        line2 = ">CAL MIN:"+String(setCalMin);
      
      }
       else if (selectSubMenu1 == 5) {
        line1 = ">CAL MAX:"+String(setCalMax);
        line2 = "                ";
      } 
      else if (selectSubMenu1 > 5) {
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
void btnRunOnEventChange(bool state) {
  stateRun = !state;
  if (stateRun) {
    timeStart = millis();
  }
}

void btnStopOnEventChange(bool state) {
  stateStop = !state;
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
    lcd.noBlink();
    lcd.noCursor();
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
    // Count
     if (selectMenu == 1 && selectSubMenu1 == 3){
        setSCW_count++;
        if(setSCW_count>50){
          setSCW_count = 1;
        }
     }else
    // CAL
     if (selectMenu == 1 && selectSubMenu1 == 2){
      selectSubMenu2++;
      if(selectSubMenu2>10){
        selectSubMenu2 = 1;
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
     if (selectMenu == 1 && selectSubMenu1 == 3){
        setSCW_count--;
        if(setSCW_count<1){
          setSCW_count = 50;
        }
     }else
     if (selectMenu == 1 && selectSubMenu1 == 2){
      selectSubMenu2--;
      if(selectSubMenu2<1){
        selectSubMenu2 = 10;
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
  } else if (selectMenu == 1 && selectSubMenu > 0 && selectSubMenu1 == 0) {
    selectSubMenu1 = 1;
    // Load data
      readArray(addressModel[indexAddressModel],setCal_std);
      int min =20000;
      int max =0;
      for (int i = 0; i < 10; i++) {
          if (setCal_std[i] < min) {
            min = setCal_std[i]; // Update min
          }
          if (setCal_std[i] > max) {
            max = setCal_std[i]; // Update max
          }
        }
      int center = (max - min) / 2;
      setCalMin = max-center;
      setCalMax = max+center;
  } else if (selectMenu == 1 && selectSubMenu > 0 && selectSubMenu1 > 0) {
    EnterSetName();
    EnterSetCalStd();
    EnterSetCountControl();
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
        // Clear lcd
        lcd.noBlink();
        lcd.noCursor();
        updateLCD("Save...........", "               ");
        // model = modelSetName;
        // Save to EEPROM
        updateEEPROM(addressModel[indexAddressModel], modelSetName);
        // Read EEPROM
        if(indexAddressModel == indexModel){
          model = readEEPROM(addressModel[indexAddressModel], lengthNameModel);
        }
        delay(1000);
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
    
      for(int i = 0; i<10; i++)
      {
         int _data = setCal_std[i];
         int address = getAddress(addressModel[indexAddressModel], i);
         // Update to EEPROM
         updateInt16ToEEPROM(address,_data);
         // 
         if(indexAddressModel == indexModel){
           UpdateCalSTD();
         }
      }
       delay(1000);
       selectSubMenu2 = 0;
    }
}

void EnterSetCountControl(){
  if(selectSubMenu1 != 3) return;

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
void updateInt16ToEEPROM(int address, int data) {
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

int readInt16CInEEPROM(int index) {
  int data = 0;
  data = EEPROM.read(index) << 8; // Read the higher byte and shift it
  data += EEPROM.read(index + 1); // Read the lower byte and add it
  return data;
}

void readArray(int indexM, int* data, int dataSize) {
  int address = indexM + 10;
  for (int i = 0;i < dataSize; i++) {
      data[i] = readInt16CInEEPROM(address);; // 
      address+=2; // Update address
  }
}
int getAddress(int indexM, int index)
{
  // int baseAddress = indexM + 10;
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