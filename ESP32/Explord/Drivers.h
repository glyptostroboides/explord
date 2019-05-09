#pragma once

/*
 * DHTesp is an optimised DHT library for the esp32
 */
#include <DHTesp.h>

/* Hardware Serial is used to define the Serial ports on the ESP32 that has three serial port
Inclusion de la bibliotheque HardWareSerial qui permet la gestion d autres ports series avec le microcontrolleur ESP32
*/
#include <HardwareSerial.h> // add the library to connect with O2 UART sensor through serial

/*
 * One Wire et Dallas Temperature sont utilisées par le DS18B20 pour la temperature
 */
#include "OneWire.h"
#include "DallasTemperature.h"

/*
 * Wire the I2C library is needed for TSL2561 and BMP280
 */
#include <Wire.h>

/* Library for the illuminance sensor */
#include <SparkFunTSL2561.h>

#include "Sensor.h"

/*
 * DHT22 Sensor : give the humidity and temperature
 */
class DHT : public Sensor {
  private:
    TempAndHumidity DHTData;
    DHTesp dht;
  public:
    DHT(): Sensor("DHT",4,&Humidity,&Temp,&Dew,&Heat){};
    virtual void init();
    virtual bool readData();
};

/*
 * LOX02 Sensor : dioxygen rate based on temperature, pressure, and ppO2
 */

class LOX : public Sensor {
  public:
    LOX(): Sensor("LOX",4,&O2,&Temp,&Pressure,&PPO2){};
    virtual void init();
    virtual bool readData();
};

/*
 * MHZ16 Sensor : carbon dioxyd rate in ppm, and temperature without decimals
 */

class MHZ : public Sensor {
  public:
    MHZ(): Sensor("MHZ",2,&CO2,&Temp,NULL,NULL){};
    virtual void init();
    virtual bool readData();
};

/*
 *  DS18B20 Sensor : give the temperature with two decimals but slow
 */

class DS : public Sensor {
  protected:
    OneWire oneWire;
    DallasTemperature tempSensor;
  public:
    DS(): oneWire(DataPin),tempSensor(&oneWire),Sensor("DS",1,&Temp,NULL,NULL,NULL){};
    virtual void init();
    virtual bool readData();   
};

/*
 * TSL2561 Sensor : give the luminosity in lux from two sensors
 */

class TSL : public Sensor {
  protected:
    SFE_TSL2561 light;
    //boolean gain; // Gain setting, 0 = X1, 1 = X16;  
    unsigned int ms; // Integration ("shutter") time in milliseconds
    unsigned int data0,data1;
    double lux;
  public:
    TSL() : Sensor("TSL",1,&Illuminance,NULL,NULL,NULL) {};
    virtual void init();
    virtual bool readData();   
};
