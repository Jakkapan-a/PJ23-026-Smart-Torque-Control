#include "DS1302.h"

DS1302 rtc(2, 3, 4); // RST, DAT, CLK

void clockdate()
{

  Serial.print(rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN, '/'));
Serial.print(" ");
  Serial.print(rtc.getDOWStr());
Serial.print(" ");
  Serial.println(rtc.getTimeStr());
}



void setup()

{

  Serial.begin(115200);

  // ตั้งเวลาครั้งแรก เอา comment นี้ออก ถ้าตั้งเสร็จแล้ว comment นี้ไว้เพื้อให้เวลาเดินต่อ

  // rtc.halt(false);
  // rtc.writeProtect(false);
  // rtc.setDOW(WEDNESDAY);         
  // rtc.setTime(11,  32,10);      
  // rtc.setDate(28, 2, 2024);    
  // rtc.writeProtect(true);
}



void loop()

{

  clockdate();

  delay(1500);
}