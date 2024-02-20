#include <ESP8266WiFi.h>
#include <PubSubClient.h>

byte macId[] = { 0xDE, 0xAD, 0xBD, 0xEF, 0xFE, 0xED };
// IPAddress ip(10, 192, 13, 173);        // IP of Arduino
// IPAddress gateway(10, 192, 13, 254);   // Gateway
// IPAddress subnet(255, 255, 255, 0);    // Subnet mask
// IPAddress primaryDNS(10, 192, 10, 5);  // DNS server
IPAddress server(10, 192, 13, 172);    // IP of MQTT server

IPAddress ip(192, 168, 137, 19);          // IP of Arduino
IPAddress gateway(192, 168, 137, 1);     // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask
IPAddress primaryDNS(192, 168, 137, 1);    // DNS server
// IPAddress server(192, 168, 137, 17);

#define ETH_CS 53
#define MQTT_MAX_PACKET_SIZE 512
int MQTT_PORT = 1883;

const char *ssid = "Internet";
const char *password = "0987654321qw";

const char *MQTT_USER = "automation";
const char *MQTT_PASS = "pssw@automation";

const String MQTT_TOPIC = "MC";
const String MQTT_TOPIC_STATUS = "status";
const String MQTT_TOPIC_SUB = "MS";
String MQTT_TOPIC_ID = "T0001";
uint32_t debounceTime = 0;
boolean statusServer = false;
const String topicPath =  String("MC/" + MQTT_TOPIC_ID).c_str();
const String topicPathStatus = String("MC/" + MQTT_TOPIC_ID + "/" + MQTT_TOPIC_STATUS).c_str();
const String topicPathSub = String("MS/" + MQTT_TOPIC_ID).c_str();


// Reconnection strategy variables
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_DELAY = 5000;      // Initial delay between reconnection attempts in milliseconds
unsigned long reconnectDelay = RECONNECT_DELAY;  // Current delay, will increase on failures

#define BUFFER_SIZE_CHAR 255  // Buffer size = 255 bytes or 255 characters
// -------------------- SERIAL 1 -------------------- //
boolean startReceived = false;
boolean endReceived = false;
const char startChar = '$';
const char endChar = '#';

// String inputString1 = "";
char receivedData[BUFFER_SIZE_CHAR];
int receivedDataLength0 = 0;
void serialEvent() {
  while (Serial.available()) {
    byte inChar = (byte)Serial.read();
    if (inChar == startChar) {
      startReceived = true;
      // inputString = "";
      memset(receivedData, 0, BUFFER_SIZE_CHAR);
      receivedDataLength0 = 0;
    } else if (startReceived && inChar == endChar) {
      endReceived = true;
    } else if (startReceived) {
      if (receivedDataLength0 < BUFFER_SIZE_CHAR - 1) {
        receivedData[receivedDataLength0++] = inChar;
      } else {
        startReceived = false;
        endReceived = false;
        receivedDataLength0 = 0;
      }
    }
  }
}
// Callback function header
void MQTT_Callback(char *topic, byte *payload, unsigned int length) {
  // Print the topic
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  // Print the payload
  Serial.print("$SUB=");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("#");
}

WiFiClient espClient;
PubSubClient client(server, MQTT_PORT, MQTT_Callback, espClient);

void setup() {
  Serial.begin(115200);
  // Serial.println("Startd");
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet, primaryDNS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  reconnect();
}

void loop() {
  manageSerial();
  uint32_t currentMillis = millis();
  if (!client.connected()) {
    // reconnect();
    if (currentMillis - lastReconnectAttempt > reconnectDelay) {
      lastReconnectAttempt = currentMillis;
      // Attempt to reconnect
      if (reconnect()) {
        reconnectDelay = RECONNECT_DELAY;  // Reset reconnect delay after successful connection
      } else {
        // Exponential backoff
        reconnectDelay = min(reconnectDelay * 2, (unsigned long)60000);
      }
    }
  } else {
    client.loop();
  }
//  Serial.println("WiFi connected");
//  delay(3000);
}
// connect to the MQTT broker function
boolean reconnect() {
  Serial.println("Attempting MQTT connection...");
  statusServer = false;

  if (client.connect(MQTT_TOPIC_ID.c_str(), MQTT_USER, MQTT_PASS, topicPathStatus.c_str(), 1, 0, "offline")) {
    // Resubscribe to the topic once reconnected
    Serial.println("Reconnected to MQTT Broker!");
    client.publish(topicPathStatus.c_str(), "online");
    client.subscribe(topicPathSub.c_str());
    statusServer = true;
    return true;
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again later");
    return false;
  }
}


void manageSerial() {
  if (startReceived && endReceived) {
    // Serial.println(inputString);
    // parseData(inputString);
    Serial.print("$ETH Received: ");
    Serial.print(receivedData);
    Serial.println("#");
    parseData(receivedData);
    Serial.println("--------0----------");

    startReceived = false;
    endReceived = false;
    // inputString = "";
    memset(receivedData, 0, BUFFER_SIZE_CHAR);
    receivedDataLength0 = 0;
  }
}

void parseData(String data) {
  data.trim();
  if (data.indexOf("STATUS_ETH:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "STATUS_ETH:");
    // Serial.println("STATUS_ETH: " + serialData);
    

    if (serialData == "ASK") {
      Serial.println("$STATUS_ETH:OK#");
    }
    
  }
  if (data.indexOf("STATUS_SERVER:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "STATUS_SERVER:");
    // Serial.println("STATUS_SERVER: " + serialData);
    if (serialData == "ASK") {
      Serial.println("$STATUS_SERVER:" + String(statusServer ? "OK" : "OFFLINE") + "#");
    }
  }

  if(statusServer == true){
    // Publish data to MQTT
    if (data.indexOf("PUB=") != -1) {
      // Send data to MES
      String serialData = extractData(data, "PUB=");
      // Serial.println("PUB= " + serialData);
      client.publish(topicPath.c_str(), serialData.c_str());
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