#include <hid.h> //Add to Oleg Mazurov code to Bar Code Scanner
#include <hiduniversal.h> //Add to Oleg Mazurov code to Bar Code Scanner
#include <usbhub.h>
#include <avr/pgmspace.h>
#include <Usb.h>
#include <usbhub.h>
#include <avr/pgmspace.h>
#include <hidboot.h>

USB Usb;
HIDUniversal Hid(&Usb); //Add this line so that the barcode scanner will be recognized, I use "Hid" below
HIDBoot<HID_PROTOCOL_KEYBOARD> Keyboard(&Usb);

class KbdRptParser : public KeyboardReportParser
{
void PrintKey(uint8_t mod, uint8_t key); // Add this line to print character in ASCII
protected:
virtual void OnKeyDown (uint8_t mod, uint8_t key);
virtual void OnKeyPressed(uint8_t key);
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  uint8_t c = OemToAscii(mod, key);
  if (c)
  OnKeyPressed(c);
}

/* what to do when symbol arrives */
void KbdRptParser::OnKeyPressed(uint8_t key)
{
  if(key!=19) Serial.print((char)key);
  else Serial.print((char)0x0D);
};
KbdRptParser Prs;

void setup()
{
  Serial.begin(9600); 
  if(Usb.Init()==-1) Serial.println("OSC did not start.");
  else Serial.println("Barcode Ready");
  Hid.SetReportParser(0,(HIDReportParser*)&Prs); //Change "Keyboard" for "Hid"
}

void loop()
{
  Usb.Task();
}