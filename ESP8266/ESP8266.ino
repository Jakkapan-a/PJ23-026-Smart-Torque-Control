// #include <ESP8266WiFi.h>
// #include <PubSubClient.h>

// const char *ssid = "Internet";
// const char *password = "0987654321qw";
// const char *mqtt_server = "192.168.137.58";
// const char *mqtt_user = "admin";
// const char *mqtt_pass = "test";
// const char *mqtt_topic = "TORQUE-1";

// WiFiClient espClient;
// PubSubClient client(espClient);

// void setup_wifi() 
// {
//   delay(10);
//   Serial.println();
//   Serial.print("Connecting to ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) 
//   {
//     delay(500);
//     Serial.print(".");
//   }
//   randomSeed(micros());
//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());
// }
// void callback(char* topic, byte* payload, unsigned int length);
  
unsigned long lastReconnectAttempt =0;



bool startReceived = false;
bool endReceived = false;

const char startChar = '$';
const char endChar = '#';
String inputString = "";

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == startChar) {
      startReceived = true;
      endReceived = false;
    } else if (startReceived && inChar == endChar) {
      endReceived = true;
    } else if (startReceived && !endReceived) {
      inputString += inChar;
    }
  }
}
void setup() 
{
    Serial.begin(115200);
    // setup_wifi();
    // client.setServer(mqtt_server, 1883);
    // client.setCallback(callback);  
}

void loop() {
  //  if (WiFi.status() == WL_CONNECTED)
  // {
  //   if (!client.connected())
  //   {
  //     reconnect();
  //   }
  //   else
  //   {
  //     client.loop();
  //   }
  // }
  handleBufferedData();
}

void handleBufferedData() {
  if (!endReceived) {
    return;
  }
  // Serial.println(inputString);
  // parseData(inputString);
  // Clear the string:
  startReceived = false;
  endReceived = false;
  // hasCompleted = false;
  inputString = "";
  // Serial response to the client is received
  Serial.println("$RECEIVED#");
}


// void parseData(String data) {
//   data.trim();
//  // publish data to MQTT
//   if (client.connected()) {
//      String topicData= "TORQUE-1/data";
//     client.publish(topicData.c_str(), data.c_str());
//   }
// }

// void reconnect()
// {
//   // Loop until we're reconnected
//   unsigned long currentMillis = millis();
//   if (currentMillis - lastReconnectAttempt > 5000)
//   {
//     lastReconnectAttempt = currentMillis;
//     // Attempt to connect
//     String clientId = "TORQUE-1";
//     String topicStatus = "TORQUE-1/status";
//     if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass,topicStatus.c_str(),0,true,"offline"))
//     {
//       Serial.println("connected");
//       // Once connected, publish an announcement...
//       String statusMess = "online";
//       client.publish(topicStatus.c_str(), statusMess.c_str(), true);
//       // ... and resubscribe
//     //   client.subscribe(mqtt_topic);
//     } 
//     else 
//     {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//     }
//   }else if(currentMillis < lastReconnectAttempt)
//   {
//     lastReconnectAttempt = currentMillis;
//   }

// }

// void callback(char* topic, byte* payload, unsigned int length) 
// {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");
//   for (int i = 0; i < length; i++) 
//   {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();
// }
