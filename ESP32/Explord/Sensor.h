#pragma once


#include "Characteristic.h"
#include "Configuration.h"

const int MaxCharacteristics = 4;

class Sensor {
  protected:
    const uint8_t PowerPin = ENPIN;
    const uint8_t DataPin = DATA1PIN;
    const uint8_t RXPin = DATA1PIN;
    const uint8_t TXPin = DATA2PIN;
    const uint8_t SDAPin = DATA1PIN;
    const uint8_t SCLPin = DATA2PIN;
    static Characteristic Temp,Humidity,Dew,Heat,O2,PPO2,Pressure,CO2,Illuminance;
    uint8_t Id;
    uint8_t CharNb;
    String Name;
    void initSerial();
    void initWire();
    Characteristic* CharSet[MaxCharacteristics]={NULL};
  public:
    Sensor(String sname, uint8_t nbchar, Characteristic* c1,Characteristic* c2,Characteristic* c3,Characteristic* c4) : 
      Name(sname), CharNb(nbchar), CharSet{c1,c2,c3,c4} {};
    String getName() { return Name; };
    void powerOn();
    void powerOff();
    virtual void init()=0;
    virtual bool readData()=0;
    void configBLEService(BLEService* pService);
    String printHeader();
    String printStringData();
    void printSerialHeader();
    void printSerialData();
    void setBLEData();
};
