#include "Log.h"

Log::Log(Sensor* pSensor, char * Path): _sensor(pSensor), current_path(Path) { }

void Log::initSD() {
  // Initialize SD card
  SPI.begin(PIN_CLK,PIN_MISO,PIN_MOSI,PIN_CS); 
  if(!SD.begin(PIN_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  // If the file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open(current_path);
  if(!file) {
    Serial.println("File doesn't exist. Creating file...");
    writeFile(_sensor->printHeader().c_str());
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
}

void Log::logSD(unsigned long * logTime) {
  Serial.print("Save data: ");
  Serial.print(_sensor->printStringData(logTime));
  appendFile(_sensor->printStringData(logTime).c_str()); 
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void Log::writeFile(const char * message) {
  Serial.printf("Writing file: %s\n", current_path);

  File file = SD.open(current_path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void Log::appendFile(const char * message) {
  Serial.printf("Appending to file: %s\n", current_path);

  File file = SD.open(current_path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void Log::readFile(){
    Serial.printf("Reading file: %s\n", current_path);

    File file = SD.open(current_path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void Log::listDir(const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = SD.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
