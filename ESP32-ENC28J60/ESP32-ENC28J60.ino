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

byte macId[] = { 0xDE, 0xAD, 0xBD, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 192, 13, 173);        // IP of Arduino
IPAddress gateway(10, 192, 13, 254);   // Gateway
IPAddress subnet(255, 255, 255, 0);    // Subnet mask
IPAddress primaryDNS(10, 192, 10, 5);  // DNS server
IPAddress server(10, 192, 13, 172);    // IP of MQTT server

// IPAddress ip(192, 168, 137, 19);          // IP of Arduino
// IPAddress gateway(192, 168, 137, 1);     // Gateway
// IPAddress subnet(255, 255, 255, 0);      // Subnet mask
// IPAddress primaryDNS(192, 168, 137, 1);    // DNS server
// IPAddress server(192, 168, 137, 17);

#define ETH_CS 53
#define MQTT_MAX_PACKET_SIZE 512
int MQTT_PORT = 1883;

const String MQTT_USER = "automation";
const String MQTT_PASS = "pssw@automation";

const String MQTT_TOPIC = "MC";
const String MQTT_TOPIC_STATUS = "status";
const String MQTT_TOPIC_SUB = "MS";
String MQTT_TOPIC_ID = "T01";
uint32_t debounceTime = 0;
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

// Reconnection strategy variables
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_DELAY = 5000;      // Initial delay between reconnection attempts in milliseconds
unsigned long reconnectDelay = RECONNECT_DELAY;  // Current delay, will increase on failures



#define BUFFER_SIZE_CHAR 255  // Buffer size = 255 bytes or 255 characters
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
      if (receivedDataLength1 < BUFFER_SIZE_CHAR - 1) {
        receivedData1[receivedDataLength1++] = inChar;
      } else {
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
  while (!Serial) continue;  // Wait for Serial port to be available
  // Start the Ethernet connection:
  // Serial.println("Initializing Ethernet...");
  // Ethernet.begin(macId, ip, primaryDNS, gateway, subnet);
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
  // connect to the MQTT broker
  // reconnect();
  // allowConnect =true;
  setUpConnect();
}

void loop() {
  uint32_t currentMillis = millis();
  manageSerial1();
  if (allowConnect == true) {
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
  }

  if (currentMillis - debounceTime > 1000) {

    Serial.print("Free RAM: ");
    Serial.println(freeRam());
    debounceTime = currentMillis;
  } else if (currentMillis < debounceTime) {
    debounceTime = currentMillis;
  }
}
// Function to check available memory
int freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
void setUpConnect() {

  Serial.println("Initializing Ethernet...");
  Ethernet.begin(macId, ip, primaryDNS, gateway, subnet);
  // Ethernet.begin(mac);
  delay(1000);

  while (Ethernet.hardwareStatus() != EthernetENC28J60) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    delay(1000);
  }
  Serial.print("Ethernet IP: ");
  Serial.println(Ethernet.localIP());
  client.setServer(server, MQTT_PORT);
  client.setCallback(MQTT_Callback);
  Serial.println("Connect to MQTT....");
  // connect to the MQTT broker
  reconnect();
  allowConnect = true;

  Serial.print("allowConnect: ");
  Serial.println(allowConnect);
}
// connect to the MQTT broker function
boolean reconnect() {
    // Loop until we're reconnected
    String topicPath = "MC/" + MQTT_TOPIC_ID + "/" + MQTT_TOPIC_STATUS;
    Serial.print("Attempting MQTT connection...");
    Serial.print("Free RAM: ");
    Serial.println(freeRam());
    statusServer = false;
  if (client.connect(MQTT_TOPIC_ID.c_str(), MQTT_USER.c_str(), MQTT_PASS.c_str(), topicPath.c_str(), 1, 0, "offline")) {
    // Resubscribe to the topic once reconnected
    Serial.println("Reconnected to MQTT Broker!");
    client.publish(topicPath.c_str(), "online");
    String topicPathSub = "MS/" + MQTT_TOPIC_ID;
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
void manageSerial1() {
  if (startReceived1 && endReceived1) {
    // Serial.println(inputString);
    // parseData(inputString);
    Serial.println(receivedData1);
    // parseData(receivedData1);
    Serial.println("--------0----------");
    startReceived1 = false;
    endReceived1 = false;
    // inputString = "";
    memset(receivedData1, 0, BUFFER_SIZE_CHAR);
    receivedDataLength1 = 0;
  }
}

#if 0
void parseData(String data) {
  data.trim();
  if (data.indexOf("STATUS_ETH:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "STATUS_ETH:");
    Serial.println("STATUS_ETH: " + serialData);
    if (serialData == "ASK") {
      Serial1.println("$STATUS_ETH:OK#");
    }
  }
  if (data.indexOf("STATUS_SERVER:") != -1) {
    // Send data to MES
    String serialData = extractData(data, "STATUS_SERVER:");
    Serial.println("STATUS_SERVER: " + serialData);
    if (serialData == "ASK") {
      Serial1.println("$STATUS_SERVER:" + String(statusServer ? "OK" : "OFFLINE") + "#");
    }
  }
  manageETH(data);
}


void manageETH(String data) {
  //   if(sendInfo == 1)
  //  {
  //    data = "ETH_ID:"+id;
  //   Serial3.println(data);
  //  }
  //  else if(sendInfo == 2)
  //  {
  //    data = "ETH_IP:"+String(IP[0])+","+String(IP[1])+","+String(IP[2])+","+String(IP[3]);
  //   Serial3.println(data);
  //  }
  //  else if(sendInfo == 3)
  //  {
  //    data = "ETH_GATEWAY:"+String(GATEWAY[0])+","+String(GATEWAY[1])+","+String(GATEWAY[2])+","+String(GATEWAY[3]);
  //   Serial3.println(data);
  //  }
  //  else if(sendInfo == 4)
  //  {
  //    data = "ETH_SUBNET:"+String(SUBNET[0])+","+String(SUBNET[1])+","+String(SUBNET[2])+","+String(SUBNET[3]);
  //  }else if(sendInfo == 5)
  //  {
  //   data = "ETH_MAC:"+String(MAC[0])+","+String(MAC[1])+","+String(MAC[2])+","+String(MAC[3])+","+String(MAC[4])+","+String(MAC[5]);
  //  }else if(sendInfo == 6)
  //  {
  //   data = "ETH_DNS:"+String(DNS[0])+","+String(DNS[1])+","+String(DNS[2])+","+String(DNS[3]);
  //  }else if(sendInfo == 7)
  //  {
  //   data = "ETH_MQTT_IP:"+String(IP_SERVER[0])+","+String(IP_SERVER[1])+","+String(IP_SERVER[2])+","+String(IP_SERVER[3]);
  //  }else if(sendInfo == 8)
  //  {
  //   data = "ETH_MQTT_PORT:"+String(SERVER_PORT_MQTT);
  //  }else if(sendInfo == 9)
  //  {
  //   data = "ETH_CONNECT:true";
  //  }

  if (data.indexOf("ETH_") == -1) {
    return;
  }

  if (data.indexOf("ETH_ID:") != -1) {
    String extract = extractData(data, "ETH_ID:");
    Serial.println("ETH_ID: " + extract);
    // SET MQTT ID
    MQTT_TOPIC_ID = extract;
    // Response OK
    Serial1.println("$ETH_ID:OK#");
    // allowConnect = false;
    // client.disconnect();

  } else if (data.indexOf("ETH_IP:") != -1) {
    String extract = extractData(data, "ETH_IP:");
    extractIP(data, "ETH_IP:", ip);
    // Response OK
    Serial1.println("$ETH_IP:OK#");
  } else if (data.indexOf("ETH_GATEWAY:") != -1) {
    String extract = extractData(data, "ETH_GATEWAY:");
    extractIP(data, "ETH_GATEWAY:", gateway);
    // Response OK
    Serial1.println("$ETH_GATEWAY:OK#");
  } else if (data.indexOf("ETH_SUBNET:") != -1) {
    String extract = extractData(data, "ETH_SUBNET:");
    extractIP(data, "ETH_SUBNET:", subnet);
    // Response OK
    Serial1.println("$ETH_SUBNET:OK#");
  } else if (data.indexOf("ETH_MAC:") != -1) {
    String extract = extractData(data, "ETH_MAC:");
    extractMac(data, "ETH_MAC:");
    // Response OK
    Serial1.println("$ETH_MAC:OK#");
  } else if (data.indexOf("ETH_DNS:") != -1) {
    String extract = extractData(data, "ETH_DNS:");
    extractIP(data, "ETH_DNS:", primaryDNS);
    // Response OK
    Serial1.println("$ETH_DNS:OK#");
  } else if (data.indexOf("ETH_MQTT_IP:") != -1) {
    String extract = extractData(data, "ETH_MQTT_IP:");
    extractIP(data, "ETH_MQTT_IP:", server);
    // Response OK
    Serial1.println("$ETH_MQTT_IP:OK#");
  } else if (data.indexOf("ETH_MQTT_PORT:") != -1) {
    String extract = extractData(data, "ETH_MQTT_PORT:");
    MQTT_PORT = extract.toInt();
    // Response OK
    Serial1.println("$ETH_MQTT_PORT:OK#");
  } else if (data.indexOf("ETH_CONNECT:") != -1) {
    String extract = extractData(data, "ETH_CONNECT:");
    Serial1.println("$ETH_CONNECT:OK#");
    // Print All data
    Serial.print("ID: ");
    Serial.println(MQTT_TOPIC_ID);
    Serial.print("IP: ");
    Serial.println(ip);
    Serial.print("Gateway: ");
    Serial.println(gateway);
    Serial.print("Subnet: ");
    Serial.println(subnet);
    Serial.print("Primary DNS: ");
    Serial.println(primaryDNS);

    Serial.print("Server: ");
    Serial.println(server);
    Serial.print("Port: ");
    Serial.println(MQTT_PORT);
    Serial.print("MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.print(macId[i], HEX);
      if (i < 5) {
        Serial.print(":");
      }
    }
    Serial.println();

    // Response OK
    Serial1.println("$ETH_CONNECT:OK#");
    // Setup connect
    setUpConnect();
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



// byte   macId[6]= {0,0,0,0,0,0};
// extract Mac address
void extractMac(String data, String key) {
  // สมมติ data = "MAC:253,254,253,253,253,252"
  int index = data.indexOf(key) + key.length();  // หาตำแหน่งเริ่มต้นของข้อมูล MAC address
  if (index > -1) {
    String macStr = data.substring(index);  // ได้ "253,254,253,253,253,252"
    int lastPos = 0, i = 0;
    while (macStr.indexOf(',', lastPos) != -1 && i < 6) {
      int pos = macStr.indexOf(',', lastPos);                   // หาตำแหน่งของ comma
      if (pos == -1) pos = macStr.length();                     // กรณีเป็นตัวเลขสุดท้าย
      macId[i] = (byte)macStr.substring(lastPos, pos).toInt();  // แปลงค่าเป็น int แล้วเก็บในอาร์เรย์
      lastPos = pos + 1;
      i++;
    }
    // กรณีตัวเลขสุดท้ายหลัง comma สุดท้าย
    if (i < 6) {
      macId[i] = (byte)macStr.substring(lastPos).toInt();
    }
  }
}

void extractIP(String data, String key, IPAddress &ipAddress) {
  // IP:10,192,13,173
  int keyIndex = data.indexOf(key);  // Find the position of the key
  if (keyIndex == -1) {
    return;  // Invalid data or key not found, so just return
  }

  int startIndex = keyIndex + key.length();   // Start index for the IP number
  String ipStr = data.substring(startIndex);  // Extract the IP string part
  int lastPos = 0, i = 0;
  byte ip[4] = { 0, 0, 0, 0 };  // Array to hold the IP parts

  // Loop to extract each part of the IP
  while (ipStr.indexOf(',', lastPos) != -1 && i < 4) {
    int pos = ipStr.indexOf(',', lastPos);  // Find the comma position
    if (pos == -1) pos = ipStr.length();    // If it's the last part of the IP

    ip[i] = (byte)ipStr.substring(lastPos, pos).toInt();  // Convert string to byte and store
    lastPos = pos + 1;                                    // Move past the comma for the next iteration
    i++;
  }

  // Handle the last part of the IP after the last comma
  if (i < 4 && lastPos < ipStr.length()) {
    ip[i] = (byte)ipStr.substring(lastPos).toInt();
  }

  // Correctly update the IPAddress object
  ipAddress = IPAddress(ip[0], ip[1], ip[2], ip[3]);
}

#endif