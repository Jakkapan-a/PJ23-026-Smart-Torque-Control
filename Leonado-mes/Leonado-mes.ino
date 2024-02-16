#include <hidboot.h>
#include <usbhub.h>
#include "Keyboard.h"
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include <TcPINOUT.h>

#define LED_STATUS 13
#define RETURN_KEY 13 // Enter key code
#define TIME_OUT_COMMUNICATION 40 // 40 seconds

#define BUFFER_SIZE_CHAR 255 // Buffer size = 255 bytes or 255 characters
char receivedData[BUFFER_SIZE_CHAR];
int receivedDataLength = 0;
boolean isDataReceived = false;
uint8_t countDownReceiveData = 0;
#define TIME_COUNT_DOWN_RECEIVE_DATA 3 // 3 seconds

// ----------------- Functions -----------------
class KbdRptParser : public KeyboardReportParser
{
    void PrintKey(uint8_t mod, uint8_t key);

  protected:
    void OnControlKeysChanged(uint8_t before, uint8_t after);

    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp	(uint8_t mod, uint8_t key);
    void OnKeyPressed(uint8_t key);
};

void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print((mod.bmLeftCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmLeftShift  == 1) ? "S" : " ");
  Serial.print((mod.bmLeftAlt    == 1) ? "A" : " ");
  Serial.print((mod.bmLeftGUI    == 1) ? "G" : " ");

  Serial.print(" >");
  PrintHex<uint8_t>(key, 0x80);
  Serial.print("< ");

  Serial.print((mod.bmRightCtrl   == 1) ? "C" : " ");
  Serial.print((mod.bmRightShift  == 1) ? "S" : " ");
  Serial.print((mod.bmRightAlt    == 1) ? "A" : " ");
  Serial.println((mod.bmRightGUI    == 1) ? "G" : " ");
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  // Serial.print("DN ");
  // PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);

  // if (c)
  //   OnKeyPressed(c);

  if (c && receivedDataLength < BUFFER_SIZE_CHAR - 1) { // check if there is a valid ASCII value and if there is space in the buffer
    receivedData[receivedDataLength++] = c;
    countDownReceiveData = TIME_COUNT_DOWN_RECEIVE_DATA;
    if (c == RETURN_KEY) {
      // Serial.println("Enter key pressed");
      receivedData[receivedDataLength] = '\0'; // terminate the string
      isDataReceived = true;
      countDownReceiveData = 0;
    }
  }
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {

  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

}
void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  // Serial.print("UP ");
  // PrintKey(mod, key);
}

void KbdRptParser::OnKeyPressed(uint8_t key)
{
  // Serial.print("ASCII: ");
  Serial.print((char)key);
};

USB     Usb;
//USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);


KbdRptParser Prs;


// ----------------- Variables -----------------
uint32_t lastDebounceTimeSecond = 0;  // the last time the output pin was toggled
uint32_t lastDebounceTimeMillis= 0;  // the last time the output pin was toggled

uint8_t CountDownCommunication = 0;

// ----------------- OUTPUT -----------------
#define LED_STATUS 13
TcPINOUT ledStatus(LED_STATUS, false);

// ----------------- Serial 1 -----------------
bool startReceived1 = false;
bool endReceived1 = false;
const char startChar1 = '$';
const char endChar1 = '#';
// String inputString1 = "";
char inputString1[BUFFER_SIZE_CHAR];
int inputString1Length = 0;
void serialEvent1() {
  while (Serial1.available()) {
    char inChar = (char)Serial1.read();
    if (inChar == startChar1) {
      startReceived1 = true;
      inputString1Length = 0;
    } else if (startReceived1 && inChar == endChar1) {
      endReceived1 = true;
    } else if (startReceived1) {
      if(inputString1Length < BUFFER_SIZE_CHAR - 1){
        inputString1[inputString1Length++] = inChar;
      }else{
        startReceived1 = false;
        endReceived1 = false;
        inputString1Length = 0;
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  #if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif
  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay( 200 );
  HidKeyboard.SetReportParser(0, &Prs);
  Keyboard.begin();
  memset(receivedData, 0, BUFFER_SIZE_CHAR); // Clear buffer
}
void loop()
{
  Usb.Task();
  if (isDataReceived) {
    Serial.print("Keyboard : ");
    Serial.println(receivedData);
    
    Serial1.print("$SERIAL:");
    Serial1.print(receivedData);
    Serial1.print("#");
  
    // 
    memset(receivedData, 0, BUFFER_SIZE_CHAR);
    receivedDataLength = 0;
    isDataReceived = false;
  }

  manageSerial1();
  uint32_t currentTime = millis();
  // ----------------- 1 Second -----------------
  if (currentTime - lastDebounceTimeSecond > 1000) {
    lastDebounceTimeSecond = currentTime;

    if(CountDownCommunication > 0){
      CountDownCommunication--;
    }

  }else if(currentTime < lastDebounceTimeSecond){
    lastDebounceTimeSecond = currentTime;
  }

  // ------------------ 0.5 Second ------------------
  if (currentTime - lastDebounceTimeMillis > 500) {
    lastDebounceTimeMillis = currentTime;
    if (CountDownCommunication >1) {
      ledStatus.toggle();
    }else if(CountDownCommunication == 1) {
      ledStatus.on();
    }

    if(countDownReceiveData > 0){
      countDownReceiveData--;
      if(countDownReceiveData == 0){
        receivedData[receivedDataLength] = '\0'; // terminate the string
        isDataReceived = true;
      }
    }

  }else if(currentTime < lastDebounceTimeMillis){
    lastDebounceTimeMillis = currentTime;
  }
  
}

void manageSerial1() {
  if (startReceived1 && endReceived1) {
    Serial.println(inputString1);
    parseData(inputString1);
    Serial.println("--------1----------");
    startReceived1 = false;
    endReceived1 = false;
    inputString1Length = 0;
    memset(inputString1, 0, BUFFER_SIZE_CHAR);
  }
}

void parseData(String data) 
{
 data.trim();
 if (data.indexOf("SERIAL:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "SERIAL:");
    Serial.println(serialData);
    // Keyboard.print(serialData);
    for(int i = 0; i < serialData.length(); i++){
      Keyboard.write(serialData[i]);
      // delay(1);
    }
    Keyboard.press(KEY_RETURN);
    // delay(1);
    Keyboard.releaseAll();
  }else if (data.indexOf("STATUS:") != -1) {
    // Send data to MES
    // String serialData = extractData(data, "STATUS:");
    Serial.println("STATUS");
    // delay(30);
    CountDownCommunication = TIME_OUT_COMMUNICATION;
    // Response to master
    Serial1.println("$STATUS_MES:OK#");
  }
}

String extractData(String data, String key) {
  int keyIndex = data.indexOf(key);  // Find the position of the key
  if (keyIndex == -1) {
    return "";  // Return 0 if key not found
  }

  int startIndex = keyIndex + key.length();      // Start index for the number
  int endIndex = data.indexOf(",", startIndex);  // Find the next comma after the key
  if (endIndex == -1) {
    endIndex = data.length();  // If no comma, assume end of string
  }

  String valueStr = data.substring(startIndex, endIndex);  // Extract the substring
  return valueStr;                                         // Convert to float and return
}
