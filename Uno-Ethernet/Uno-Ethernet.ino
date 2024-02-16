// #include <Wire.h>
#include <SPI.h>
// #include <SD.h>
#include <UIPEthernet.h>
// #include <Ethernet.h>
#include <PubSubClient.h>
/*
ENC28J60:
CS  -> 10
SI  -> 11
SO  -> 12
SCK -> 13
RST -> 8
*/

byte mac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };
IPAddress ip(10, 192, 13, 180);          // IP of Arduino
IPAddress gateway(10, 192, 13, 254);     // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask
IPAddress primaryDNS(10, 192, 10, 5);    // DNS server
IPAddress secondaryDNS(10, 192, 10, 6);  // DNS server
IPAddress server(10, 192, 13, 172);      // IP of MQTT server
#define ETH_CS 53
#define MQTT_PORT 1883
const String MQTT_USER = "automation";
const String MQTT_PASS = "pssw@automation";
const String MQTT_TOPIC = "MC";
const String MQTT_TOPIC_ID = "PJ23-001-1";
const String MQTT_TOPIC_STATUS = "status";
const String MQTT_TOPIC_SUB = "receive";

// Callback function header
void MQTT_Callback(char *topic, byte *payload, unsigned int length);

EthernetClient ethClient;
// PubSubClient client(ethClient);
PubSubClient client(server, 1883, MQTT_Callback, ethClient);

// -------------------- I2c -------------------- //
boolean startReceivedI2c = false;
boolean endReceivedI2c = false;

const char startCharI2c = '$';
const char endCharI2c = '#';
String inputStringI2c = "";



// -------------------- SERIAL  -------------------- //
boolean startReceived = false;
boolean endReceived = false;

const char startChar = '$';
const char endChar = '#';
String inputString = "";

void serialEvent() {
  while (Serial.available()) {
    byte inChar = (byte)Serial.read();
    if (inChar == startChar) {
      startReceived = true;
      inputString = "";
      // inputString += (char)inChar;
    } else if (startReceived && inChar == endChar) {
      // inputString += (char)inChar;
      endReceived = true;
    } else if (startReceived) {
      inputString += (char)inChar;
    }
  }
}

void setup() {
  Serial.begin(115200);

  Ethernet.begin(mac, ip, primaryDNS, gateway, subnet);

  while(Ethernet.hardwareStatus() != EthernetENC28J60){
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    delay(1000);
  }

  Serial.println("ethClient connected");

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  if (Ethernet.linkStatus() == LinkON) {
    Serial.println("Ethernet cable is connected.");
  }
  Serial.print("IP Address: ");
  Serial.print(Ethernet.localIP());
  Serial.print(" Subnet Mask: ");
  Serial.print(Ethernet.subnetMask());
  Serial.print(" Gateway: ");
  Serial.print(Ethernet.gatewayIP());
  Serial.print(" DNS: ");
  Serial.print(Ethernet.dnsServerIP());
  Serial.print(" State : ");
  Serial.print(Ethernet.linkStatus());
  Serial.println();
 
  reconnect();
  client.setCallback(MQTT_Callback);

  Serial.println("Start");
  delay(1000);

}

void loop() {
  manageSerial();
  manageI2c();
  if (!client.connected()) {
    // Connect MQTT server
    // if (!ConnectMQTT()) {
    //   return;
    // }
    reconnect();
  } else {
    client.loop();
  }
}

void MQTT_Callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  String topicPath = "MC/" + String(MQTT_TOPIC_ID) + "/" + MQTT_TOPIC_STATUS;
  Serial.print("Connecting to MQTT server: ");
  while (!client.connected()){
    if (client.connect(MQTT_TOPIC_ID.c_str(), MQTT_USER.c_str(), MQTT_PASS.c_str(), topicPath.c_str(), 0, true, "offline")) {
      Serial.println("Connected to MQTT server");
      client.publish(topicPath.c_str(), "online");
      Serial.println("Publish status online");

      String topicPath = "MC/" + String(MQTT_TOPIC_ID) + "/" + MQTT_TOPIC_SUB;
      Serial.print("Subscribe to: ");
      Serial.println(topicPath);
      client.subscribe(topicPath.c_str());
      delay(500);
    } else {
      Serial.println("MQTT disconnect");
      delay(1000);
    }   
  }
  // if (client.connect(MQTT_TOPIC_ID.c_str(), MQTT_USER.c_str(), MQTT_PASS.c_str(), topicPath.c_str(), 0, true, "offline")) {
  //   Serial.println("Connected to MQTT server");
  //   client.publish(topicPath.c_str(), "online");
  //   Serial.println("Publish status online");
  //   delay(500);
  //   return true;
  // } else {
  //   // Connecting
  //   Serial.println("MQTT disconnect");
  //   return false;
  // }
}

void manageSerial() {
  if (startReceived && endReceived) {
    Serial.println(inputString);
    parseData(inputString);
    Serial.println("--------0----------");
    startReceived = false;
    endReceived = false;
    inputString = "";
  }
}

void manageI2c() {
  if (startReceivedI2c && endReceivedI2c) {
    Serial.println(inputStringI2c);
    parseData(inputStringI2c);
    Serial.println("--------1----------");
    startReceivedI2c = false;
    endReceivedI2c = false;
    inputStringI2c = "";
  }
}

void parseData(String data) {
  data.trim();
  if (data.indexOf("SERIAL:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "SERIAL:");
    Serial.print("REC:");
    Serial.println(serialData);
    // Keyboard.print(serialData);

  } else if (data.indexOf("STATUS:") != -1) {
    // Send data to MES
    // String serialData = extractData(data, "STATUS:");
    Serial.println("STATUS");
    // delay(30);
    // CountDownCommunication = TIME_OUT_COMMUNICATION;
    // Response to master
    Serial.println("$STATUS_UNO:OK#");
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

// void receiveEvent(int howMany) {
//   while (Wire.available()) {  // loop through all but the last
//     char c = Wire.read();     // receive byte as a character
//     Serial.print(c);          // print the character
//     if (c == startCharI2c) {
//       startReceivedI2c = true;
//       inputStringI2c = "";
//     } else if (startReceivedI2c && c == endCharI2c) {
//       endReceivedI2c = true;
//     } else if (startReceivedI2c) {
//       inputStringI2c += c;
//     }
//   }
//   Serial.println();
// }

// void requestEvent() {
//   Wire.write("x");  // respond with message of 1 byte
// }