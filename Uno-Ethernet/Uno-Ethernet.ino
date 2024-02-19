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

byte mac[] = { 0xEB, 0x02, 0xFF, 0xFA, 0x3A, 0x33 };
IPAddress ip(10, 192, 13, 180);          // IP of Arduino
IPAddress gateway(10, 192, 13, 254);     // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask
IPAddress primaryDNS(10, 192, 10, 5);    // DNS server
IPAddress secondaryDNS(10, 192, 10, 6);  // DNS server
IPAddress server(10, 192, 13, 172);      // IP of MQTT server
#define ETH_CS 53
#define MQTT_PORT 1883
#define MQTT_MAX_PACKET_SIZE 512

const String MQTT_USER = "automation";
const String MQTT_PASS = "pssw@automation";
const String MQTT_TOPIC = "MC";
const String MQTT_TOPIC_ID = "PJ23-001-1";
const String MQTT_TOPIC_STATUS = "status";
const String MQTT_TOPIC_SUB = "MS";

// Callback function header
void MQTT_Callback(char *topic, byte *payload, unsigned int length);

EthernetClient ethClient;
// PubSubClient client(ethClient);
PubSubClient client(server, MQTT_PORT, MQTT_Callback, ethClient);

// -------------------- I2c -------------------- //
boolean startReceivedI2c = false;
boolean endReceivedI2c = false;

const char startCharI2c = '$';
const char endCharI2c = '#';
String inputStringI2c = "";

#define BUFFER_SIZE_CHAR 255 // Buffer size = 255 bytes or 255 characters
// -------------------- SERIAL 1 -------------------- //
boolean startReceived1 = false;
boolean endReceived1 = false;

const char startChar1 = '$';
const char endChar1 = '#';
// String inputString1 = "";
char receivedData1[BUFFER_SIZE_CHAR];
int receivedDataLength1 = 0;
void serialEvent1() {
  while (Serial1.available()) {
    byte inChar = (byte)Serial1.read();
    if (inChar == startChar1) {
      startReceived1 = true;
      // inputString = "";
      memset(receivedData1, 0, BUFFER_SIZE_CHAR);
      receivedDataLength1 = 0;
      // inputString += (char)inChar;
    } else if (startReceived1 && inChar == endChar1) {
      // inputString += (char)inChar;
      endReceived1 = true;
    } else if (startReceived1) {
      // inputString += (char)inChar;
      if(receivedDataLength1 < BUFFER_SIZE_CHAR - 1){
        receivedData1[receivedDataLength1++] = inChar;
      }else{
        startReceived1 = false;
        endReceived1 = false;
        receivedDataLength1 = 0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  Ethernet.begin(mac, ip, primaryDNS, gateway, subnet);
  // Not set IP
  // Ethernet.begin(mac);
  delay(1000); // รอการเชื่อมต่อเครือข่าย

  while (Ethernet.hardwareStatus() != EthernetENC28J60) {
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
  Serial.print(Ethernet.linkStatus() == LinkON ? "Connected" : "Disconnected");
  Serial.println();
  
  // client.setServer(server, MQTT_PORT);
  client.setKeepAlive(60); // ตั้งค่าเวลา keepalive เป็น 60 วินาที
  client.setBufferSize(MQTT_MAX_PACKET_SIZE);
  reconnect();
  client.setCallback(MQTT_Callback);

  Serial.println("Start");
  delay(1000);
}

void loop() {
  manageSerial1();
  manageI2c();
  if (!client.connected()) {
    reconnect();
  } else {
    client.loop();
  }
   Serial.print("Task");
   delay(500);
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
boolean toggleDNS = false;
void reconnect() {
  String topicPath = "MC/" + MQTT_TOPIC_ID + "/" + MQTT_TOPIC_STATUS;
  Serial.print("Connecting to MQTT server: ");
  while (!client.connected()) {
      Serial.print("IP Address: ");
      Serial.print(Ethernet.localIP());
      Serial.print(" Subnet Mask: ");
      Serial.print(Ethernet.subnetMask());
      Serial.print(" Gateway: ");
      Serial.print(Ethernet.gatewayIP());
      Serial.print(" DNS: ");
      Serial.print(Ethernet.dnsServerIP());
      Serial.print(" State : ");
      Serial.print(Ethernet.linkStatus() == LinkON ? "Connected" : "Disconnected");
      Serial.println();

//       Change DNS
//       if(toggleDNS){
//         Ethernet.begin(mac, ip, primaryDNS, gateway, subnet);
//       }else{
//         Ethernet.begin(mac, ip, secondaryDNS, gateway, subnet);
//       }
//      toggleDNS = !toggleDNS;

      if(Ethernet.linkStatus() == LinkOFF){
        return;
      }
    //  client.setServer(server, MQTT_PORT);
     Serial.print("MQTT server: ");
     Serial.println(client.state());
     Serial.println("Attempting MQTT connection...");
    if (client.connect(MQTT_TOPIC_ID.c_str(), MQTT_USER.c_str(), MQTT_PASS.c_str(), topicPath.c_str(), 0, true, "offline")) {
      Serial.println("Connected to MQTT server");
      client.publish(topicPath.c_str(), "online");
      Serial.println("Publish status online");

      topicPath = "MS/" + MQTT_TOPIC_ID;
      Serial.print("Subscribe to: ");
      Serial.println(topicPath);
      client.subscribe(topicPath.c_str());
      delay(500);
      break;  
    } else {
      Serial.println("MQTT disconnect");
      // delay(1000);
    }
  }
}
void manageSerial1() {
  if (startReceived1 && endReceived1) {
    // Serial.println(inputString);
    // parseData(inputString);
    Serial.println(receivedData1);
    parseData(receivedData1);
    Serial.println("--------0----------");
    startReceived1 = false;
    endReceived1 = false;
    // inputString = "";
    memset(receivedData1, 0, BUFFER_SIZE_CHAR);
    receivedDataLength1 = 0;
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