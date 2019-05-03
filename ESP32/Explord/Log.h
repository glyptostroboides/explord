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
      void writeFile(const char * path, const char * message);
      void appendFile(const char * path, const char * message);
      Sensor* _sensor;
    public:
      Log(Sensor* pSensor): _sensor(pSensor) {};
      void initSD();
      void logSD();
      void readFile(const char * path);
      void listDir(const char * dirname, uint8_t levels);
};
