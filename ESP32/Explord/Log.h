#pragma once

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "Sensor.h"

#define PIN_MISO 2
#define PIN_MOSI 15
#define PIN_CLK 14
#define PIN_CS 13

class Log {
    private:
      //String CurrentPath;
      const char * current_path; 
      void writeFile(const char * message);
      void appendFile(const char * message);
      Sensor* _sensor;
    public:
      Log(Sensor* pSensor, const char * Path);
      void initSD();
      void logSD();
      void readFile();
      void listDir(const char * dirname, uint8_t levels);
};
