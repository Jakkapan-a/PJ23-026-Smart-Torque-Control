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
// IPAddress ip(10, 192, 13, 173);          // IP of Arduino
// IPAddress gateway(10, 192, 13, 254);     // Gateway
// IPAddress subnet(255, 255, 255, 0);      // Subnet mask
// IPAddress primaryDNS(10, 192, 13, 172);    // DNS server

IPAddress ip(192, 168, 137, 19);          // IP of Arduino
IPAddress gateway(192, 168, 137, 1);     // Gateway
IPAddress subnet(255, 255, 255, 0);      // Subnet mask
IPAddress primaryDNS(192, 168, 137, 1);    // DNS server

IPAddress secondaryDNS(10, 192, 10, 6);  // DNS server
// IPAddress server(10, 192, 13, 172);      // IP of MQTT server

IPAddress server(192, 168, 137, 17);
#define ETH_CS 53
#define MQTT_PORT 1883
#define MQTT_MAX_PACKET_SIZE 512

const String MQTT_USER = "automation";
const String MQTT_PASS = "pssw@automation";
const String MQTT_TOPIC = "MC";
const String MQTT_TOPIC_ID = "T01";
const String MQTT_TOPIC_STATUS = "status";
const String MQTT_TOPIC_SUB = "MS";

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

void setup() {
  Serial.begin(115200);
  while (!Serial) continue; // Wait for Serial port to be available
  // Start the Ethernet connection:
  Serial.println("Initializing Ethernet...");
  Ethernet.begin(mac, ip, primaryDNS, gateway, subnet);
  // Ethernet.begin(mac);
  delay(1000); // รอการเชื่อมต่อเครือข่าย

  while (Ethernet.hardwareStatus() != EthernetENC28J60) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    delay(1000);
  }

  Serial.print("Ethernet IP: ");
  Serial.println(Ethernet.localIP());
  
  client.setServer(server, MQTT_PORT);
  client.setCallback(MQTT_Callback);
  
  Serial.println("Connect to MQTT....");
  String topicPath = "MC/" + MQTT_TOPIC_ID + "/" + MQTT_TOPIC_STATUS;

  // Attempt to connect to the MQTT broker with the given username and password
  if (client.connect("arduinoClient", MQTT_USER.c_str(), MQTT_PASS.c_str(), topicPath.c_str(), 1, 0, "offline"))
  {
    Serial.println("Connected to MQTT Broker!");
    // online
    client.publish(topicPath.c_str(), "online");

    String topicPathSub = "MS/" + MQTT_TOPIC_ID;
    client.subscribe(topicPathSub.c_str());  } else {
    Serial.println("MQTT connection failed. Check MQTT broker availability and credentials.");
  }
}

void loop() {
if (!client.connected()) {
  Serial.println("MQTT connection lost. Attempting to reconnect...");
    // Reconnect if connection is lost
    String topicPath = "MC/" + MQTT_TOPIC_ID + "/" + MQTT_TOPIC_STATUS;
    if (client.connect("arduinoClient", MQTT_USER.c_str(), MQTT_PASS.c_str(), topicPath.c_str(), 1, 0, "offline")) 
    {
          // online
      client.publish(topicPath.c_str(), "online");
      // Resubscribe to the topic once reconnected
      Serial.println("Reconnected to MQTT Broker!");
      String topicPathSub = "MS/" + MQTT_TOPIC_ID;
      client.subscribe(topicPathSub.c_str());
    }
  }
  client.loop();
}