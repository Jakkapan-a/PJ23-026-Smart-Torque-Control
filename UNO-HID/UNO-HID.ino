#include <hidboot.h>
#include <usbhub.h>
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include <TcPINOUT.h>

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

  if (c)
    OnKeyPressed(c);
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {

  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  // if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl) {
  //   Serial.println("LeftCtrl changed");
  // }
  // if (beforeMod.bmLeftShift != afterMod.bmLeftShift) {
  //   Serial.println("LeftShift changed");
  // }
  // if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt) {
  //   Serial.println("LeftAlt changed");
  // }
  // if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI) {
  //   Serial.println("LeftGUI changed");
  // }

  // if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl) {
  //   Serial.println("RightCtrl changed");
  // }
  // if (beforeMod.bmRightShift != afterMod.bmRightShift) {
  //   Serial.println("RightShift changed");
  // }
  // if (beforeMod.bmRightAlt != afterMod.bmRightAlt) {
  //   Serial.println("RightAlt changed");
  // }
  // if (beforeMod.bmRightGUI != afterMod.bmRightGUI) {
  //   Serial.println("RightGUI changed");
  // }

}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  // Serial.print("UP ");
  // PrintKey(mod, key);
}

String receivedData = "";
uint8_t returnKey = 13; // Enter key code is 13
boolean isDataReceived = false;
void KbdRptParser::OnKeyPressed(uint8_t key)
{
  // Serial.print((char)key);
  if (key == returnKey) {
    isDataReceived = true;
  } else {
    receivedData += (char)key;
  }
};

USB     Usb;
// USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

KbdRptParser Prs;
// ----------------- OUTPUT -----------------
#define LED_STATUS 13
TcPINOUT ledStatus(LED_STATUS, false);

// ----------------- Variables -----------------
uint32_t lastDebounceTimeSecond = 0;  // the last time the output pin was toggled
uint32_t lastDebounceTimeMillis= 0;  // the last time the output pin was toggled

uint8_t CountDownCommunication = 0;
const uint8_t TIME_OUT_COMMUNICATION = 40; // 30 seconds

// ----------------- Serial 1 -----------------
bool startReceived1 = false;
bool endReceived1 = false;
const char startChar1 = '$';
const char endChar1 = '#';
String inputString1 = "";
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if ((char)inChar == startChar1) {
      startReceived1 = true;
      inputString1 = "";
      // inputString1 += String(inChar, DEC);
    } else if (startReceived1 && (char)inChar == endChar1) {
      // inputString1 += String(inChar, DEC);
      endReceived1 = true;
    } else if (startReceived1 && !endReceived1) {
      inputString1 += (char)inChar;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  // Serial1.begin(115200);
  #if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif
  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay( 200 );
  HidKeyboard.SetReportParser(0, &Prs);
}
void loop()
{
  Usb.Task();

  if (isDataReceived) {
    Serial.print("Keyboard : ");
    Serial.println(receivedData);
    // Serial.println("$SERIAL:" + receivedData+"#");
    // Serial1.println("$SERIAL:" + receivedData+"#");
    Serial.write("$SERIAL:");
    for(int i = 0; i < receivedData.length(); i++){
      Serial.write(receivedData[i]);
    }
    Serial.write("#");
    receivedData = "";
    isDataReceived = false;
    delay(1);
  }

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
    if (CountDownCommunication > 0) {
      ledStatus.toggle();
 
    }else{
      ledStatus.on();
    }
  }else if(currentTime < lastDebounceTimeMillis){
    lastDebounceTimeMillis = currentTime;
  }

  manageSerial1();
}

void manageSerial1() {
  if (startReceived1 && endReceived1) {
    Serial.println(inputString1);
    parseData(inputString1);
    Serial.println("--------1----------");
    startReceived1 = false;
    endReceived1 = false;
    inputString1 = "";
  }
}

void parseData(String data) 
{
 data.trim();
 if (data.indexOf("SERIAL:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "SERIAL:");
    Serial.print("REC:");
    Serial.println(serialData);
    // Keyboard.print(serialData);
    for(int i = 0; i < serialData.length(); i++){
      // Keyboard.write(serialData[i]);
      // delay(1);
    }
    // Keyboard.press(KEY_RETURN);
    // delay(1);
    // Keyboard.releaseAll();
  }else if (data.indexOf("STATUS:") != -1) {
    // Send data to MES
    // String serialData = extractData(data, "STATUS:");
    Serial.println("STATUS");
    // delay(30);
    CountDownCommunication = TIME_OUT_COMMUNICATION;
    // Response to master
    Serial.println("$STATUS_MES:OK#");
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
