#include <SPI.h>
#include <UIPEthernet.h>
// #include <Ethernet.h>
#include <PubSubClient.h>
/*
ENC28J60: Mega2560
CS  -> 53
SI  -> 51
SO  -> 50
SCK -> 52
RST -> 8
*/

byte mac[] = { 0xDE, 0xAD, 0xBD, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 192, 13, 173);          // IP of Arduino
IPAddress gateway(10, 192, 13, 254);     // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask
IPAddress primaryDNS(10, 192, 13, 172);    // DNS server

// IPAddress ip(192, 168, 137, 19);          // IP of Arduino
// IPAddress gateway(192, 168, 137, 1);     // Gateway
// IPAddress subnet(255, 255, 255, 0);      // Subnet mask
// IPAddress primaryDNS(192, 168, 137, 1);    // DNS server

IPAddress secondaryDNS(10, 192, 10, 6);  // DNS server
IPAddress server(10, 192, 13, 172);      // IP of MQTT server

// IPAddress server(192, 168, 137, 17);
#define ETH_CS 53
#define MQTT_PORT 1883
#define MQTT_MAX_PACKET_SIZE 512

const String MQTT_USER = "automation";
const String MQTT_PASS = "pssw@automation";

const String MQTT_TOPIC = "MC";
const String MQTT_TOPIC_STATUS = "status";
const String MQTT_TOPIC_SUB = "MS";
String MQTT_TOPIC_ID = "T0001";

boolean statusServer = false;
// Callback function header
void MQTT_Callback(char *topic, byte *payload, unsigned int length) {
  // Print the topic
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // Print the payload
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

EthernetClient ethClient;
// PubSubClient client(ethClient);
PubSubClient client(server, MQTT_PORT, MQTT_Callback, ethClient);
boolean allowConnect = false;

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
  Serial1.begin(115200);
  while (!Serial) continue; // Wait for Serial port to be available
  // Start the Ethernet connection:
  // Serial.println("Initializing Ethernet...");
  // Ethernet.begin(mac, ip, primaryDNS, gateway, subnet);
  // // Ethernet.begin(mac);
  // delay(1000); 

  // while (Ethernet.hardwareStatus() != EthernetENC28J60) {
  //   Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  //   delay(1000);
  // }
  // Serial.print("Ethernet IP: ");
  // Serial.println(Ethernet.localIP());
  // client.setServer(server, MQTT_PORT);
  // client.setCallback(MQTT_Callback);
  // Serial.println("Connect to MQTT....");
  // // connect to the MQTT broker
  // reconnect();
}

void loop() {

  manageSerial1();
  if(allowConnect == true){
    if (!client.connected()) {
        reconnect();
      }
      client.loop();
  }


}

void setUpConnect(){
  Ethernet.begin(mac, ip, primaryDNS, gateway, subnet);
   delay(1000); 
   
  while (Ethernet.hardwareStatus() != EthernetENC28J60) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    delay(1000);
  }

  client.setServer(server, MQTT_PORT);
  client.setClient(ethClient);
  Serial.print("Ethernet IP: ");
  Serial.println(Ethernet.localIP());
  client.setServer(server, MQTT_PORT);
  client.setCallback(MQTT_Callback);
  Serial.println("Connect to MQTT....");
  // connect to the MQTT broker
  reconnect();
}
// connect to the MQTT broker function
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
     String topicPath = "MC/" + MQTT_TOPIC_ID + "/" + MQTT_TOPIC_STATUS;
    if (client.connect(MQTT_TOPIC_ID.c_str(), MQTT_USER.c_str(), MQTT_PASS.c_str(), topicPath.c_str(), 1, 0, "offline")) 
    {
      // Resubscribe to the topic once reconnected
      Serial.println("Reconnected to MQTT Broker!");
      client.publish(topicPath.c_str(), "online", true);
      String topicPathSub = "MS/" + MQTT_TOPIC_ID;
      client.subscribe(topicPathSub.c_str());
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      // Wait 3 seconds before retrying
      delay(3000);
    }
  }
}
void manageSerial1() 
{
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

void parseData(String data) {
  data.trim();
 if (data.indexOf("STATUS_ETH:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "STATUS_ETH:");
    Serial.println("STATUS_ETH: " + serialData);
    if(serialData == "ASK"){
      Serial1.println("$STATUS_ETH:OK#");
    }
  }
  if (data.indexOf("STATUS_SERVER:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "STATUS_SERVER:");
    Serial.println("STATUS_SERVER: " + serialData);
    if(serialData == "ASK"){
      Serial1.println("$STATUS_SERVER:"+String(statusServer?"OK":"OFFLINE")+"#");
    }
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

IPAddress extractIP(String data, String key) {
  int keyIndex = data.indexOf(key);  // Find the position of the key
  if (keyIndex == -1) {
    return IPAddress(0, 0, 0, 0);  // Return 0 if key not found
  }

  int startIndex = keyIndex + key.length();      // Start index for the number
  int endIndex = data.indexOf(",", startIndex);  // Find the next comma after the key
  if (endIndex == -1) {
    endIndex = data.length();  // If no comma, assume end of string
  }

  String valueStr = data.substring(startIndex, endIndex);  // Extract the substring
  return IPAddress(valueStr.toInt());                                         // Convert to float and return
} 