#include <SPI.h>
#include <SD.h>

#define SD_CS 4
// -------------------- SD CARD -------------------- //
#define FILE_NAME_SIZE 11
char fileName[FILE_NAME_SIZE];
int fileIndex = 0;
uint32_t totalNumberFile = 0;
#define BUFFER_SIZE_DATA 255
boolean isReadNumberFile = false;
// -------------------- SERIAL  -------------------- //
boolean startReceived = false;
boolean endReceived = false;

const char startChar = '$';
const char endChar = '#';
// String inputString = "";
char inputString[BUFFER_SIZE_DATA];
int inputStringLength = 0;
void serialEvent() {
  while (Serial.available()) {
    byte inChar = (byte)Serial.read();
    if (inChar == startChar) {
      startReceived = true;
      inputStringLength = 0;
    } else if (startReceived && inChar == endChar) {
      endReceived = true;
    } else if (startReceived) {
      if (inputStringLength < BUFFER_SIZE_DATA - 1) {
        inputString[inputStringLength++] = inChar;
      } else {
        startReceived = false;
        endReceived = false;
        inputStringLength = 0;
      }
    }
  }
}

void setup() {

  Serial.begin(115200);
  pinMode(SS, OUTPUT);
  memset(inputString, 0, BUFFER_SIZE_DATA);
checkSDCard();
  isReadNumberFile = true;
}

uint32_t lastTime = 0;

void loop() {
  manageSerial();

  uint32_t currentTime = millis();
  if (currentTime - lastTime > 2000) {
    lastTime = currentTime;
    checkSDCard();
  } else if (currentTime < lastTime) {
    lastTime = currentTime;
  }

  if(isReadNumberFile){
    totalNumberFile = getTotalNumberOfFiles();
    isReadNumberFile = false;
     generateFileName();
  }
}

void manageSerial() {
  if (startReceived && endReceived) {
    inputString[inputStringLength] = '\0';
    // Serial.println(inputString);
    parseData(inputString);
    startReceived = false;
    endReceived = false;
    memset(inputString, 0, BUFFER_SIZE_DATA);
    inputStringLength = 0;
  }
}

void parseData(String data) {
  if (data.indexOf("WRITE:") != -1) {
    String value = extractData(data, "WRITE:");
    // CUT LIMIT SIZE OF FILE
    if (value.length() > 255) {
      value = value.substring(0, 255);
    }
    Serial.print("->");
    Serial.print(fileName);
    Serial.print("->");
    Serial.println(value);
    appendFile(fileName, value.c_str());
    Serial.println("$WRITE:OK#");
  }else if (data.indexOf("NEW_FILE") != -1) {
    totalNumberFile++;
    generateFileName();
    Serial.println("$NEW_FILE:OK#");
  } 
}


String extractData(String data, String key) {
  int keyIndex = data.indexOf(key);  // Find the position of the key
  if (keyIndex == -1) {
    return "";  // Return 0 if key not found
  }

  int startIndex = keyIndex + key.length();      // Start index for the number
  // int endIndex = data.indexOf(",", startIndex);  // Find the next comma after the key
  // if (endIndex == -1) {
  // }
  
  endIndex = data.length();  // If no comma, assume end of string

  String valueStr = data.substring(startIndex, endIndex);  // Extract the substring
  return valueStr;                                         // Convert to float and return
}
void generateFileName() {

  int indexName = 0;
  // Clear file
  memset(fileName, 0, FILE_NAME_SIZE);
  fileName[indexName++] = 'T';
  String str = String(totalNumberFile);
  // Add to file name
  for (int i = 0; i < str.length(); i++) {
    fileName[indexName++] = str[i];
  }
  // Add file extension
  strcat(fileName, ".txt");
  fileName[strlen(fileName)] = '\0';  // terminate the string

  Serial.print("->");
  Serial.print(fileName);
  Serial.println("");
}


void checkSDCard() {
  while (!SD.begin(SD_CS)) {
    Serial.println("$SD_CARD:ERROR#");
    delay(1000);
    isReadNumberFile = true;
  }
  Serial.println("$SD_CARD:OK#");
}


uint32_t getTotalNumberOfFiles() {
  uint32_t totalFiles = 0;
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    if (entry.isDirectory()) {
      // skip directories
      continue;
    }
    totalFiles++;
    entry.close();
  }
  root.close();
  return totalFiles;
}


void readSDCard(const char *filename) {
  // Open file for reading data and BASE_PATH
  File file = SD.open(filename);
  if (file) {
    Serial.println("Read from file:");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }
}

void appendFile(const char *filename, const char *message) {
  // Open file for appending data
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.println(message);
    file.close();
  } else {
    Serial.println("Failed to open file for appending");
  }
}
