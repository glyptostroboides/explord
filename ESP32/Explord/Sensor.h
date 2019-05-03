#pragma once


#include "Characteristic.h"

const int MaxCharacteristics = 4;



class Sensor {
  protected:
    const uint8_t PowerPin = 23;
    const uint8_t DataPin = 19;
    const uint8_t RXPin = 19;
    const uint8_t TXPin = 21;
    static Characteristic Temp,Humidity,Dew,Heat,O2,PPO2,Pressure,CO2;
    uint8_t Id;
    uint8_t CharNb;
    String Name;
    void initSerial();
    Characteristic* CharSet[MaxCharacteristics]={NULL};
  public:
    //Sensor(uint8_t sensor_id): Id(sensor_id) {};
    Sensor(String sname, uint8_t nbchar, Characteristic* c1,Characteristic* c2,Characteristic* c3,Characteristic* c4) : 
      Name(sname), 
      CharNb(nbchar),
      CharSet{c1,c2,c3,c4}
     {};
    //void addCharacteristic(Characteristic*,uint8_t);
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