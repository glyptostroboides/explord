#pragma once

#include "Sensor.h"
#include <DHTesp.h>
/* Hardware Serial is used to define the Serial ports on the ESP32 that has three serial port
  // Inclusion de la bibliotheque HardWareSerial qui permet la gestion d autres ports series avec le microcontrolleur ESP32*/
#include <HardwareSerial.h> // add the library to connect with O2 UART sensor through serial

class DHT : public Sensor {
  private:
    TempAndHumidity DHTData;
    DHTesp dht;
  public:
    DHT(): Sensor("DHT",4,&Humidity,&Temp,&Dew,&Heat){};
    virtual void init();
    virtual bool readData();
};

class LOX : public Sensor {
  public:
    LOX(): Sensor("LOX",4,&O2,&Temp,&Pressure,&PPO2){};
    virtual void init();
    virtual bool readData();
};

class MHZ : public Sensor {
  public:
    MHZ(): Sensor("MHZ",2,&CO2,&Temp,NULL,NULL){};
    virtual void init();
    virtual bool readData();
};
