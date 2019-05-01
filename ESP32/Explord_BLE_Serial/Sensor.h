#pragma once

#include <DHTesp.h>
/* Hardware Serial is used to define the Serial ports on the ESP32 that has three serial port
// Inclusion de la bibliotheque HardWareSerial qui permet la gestion d autres ports series avec le microcontrolleur ESP32*/
#include <HardwareSerial.h> // add the library to connect with O2 UART sensor through serial
#include "Characteristic.h"
/*
#include "DHT22Sensor.h"
#include "MHZ16Sensor.h"
#include "LOX02Sensor.h"
*/
const int MaxCharacteristics = 4;

class Sensor {
  private:
    uint8_t Id;
    uint8_t CharNb;
    String Name;
    const uint8_t PowerPin=4;
    const uint8_t DataPin=17;
    const uint8_t RXPin=19;
    const uint8_t TXPin=21;
    Characteristic* CharSet[MaxCharacteristics];
    DHTesp dht;
  public:
    Sensor(uint8_t sensor_id):Id(sensor_id){};
    void powerOn();
    void init();
    String getName(){return Name;};
    bool getData();
    void configBLEService(BLEService* pService);
    void printSerialHeader();
    void printSerialData();     
    void setBLEData();   
};
