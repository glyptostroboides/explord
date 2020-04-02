#pragma once

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "Sensor.h"

#include "Configuration.h"

class Log {
    private:
      //String CurrentPath;
      char * current_path; 
      void writeFile(const char * message);
      void appendFile(const char * message);
      Sensor* _sensor;
    public:
      Log(Sensor* pSensor, char * Path);
      void initSD();
      void logSD(unsigned long * logTime);
      void readFile();
      void listDir(const char * dirname, uint8_t levels);
};
