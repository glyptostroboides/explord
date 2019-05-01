#pragma once

/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "Characteristic.h"

#include <DHTesp.h>
  
class DHT22Sensor {
  private :
    const String DHTSensorName = "DHT";
    /*
    * Define the pins used for the datas and power of DHT22
    * Définition de la broche utilisée pour les données du DHT22
    */
    const uint8_t DHTDataPin = 17;
    const uint8_t DHTPowerPin = 4;
    DHTesp dht;
    //TempAndHumidity DHTData;
    /*Store the integer from DHT*/
    //uint16_t dHumidity;
    //int16_t dTemp, dHeat,dDew;
    /*Store the values sended by DHT as strings*/
    //String sTemp, sHumidity, sHeat, sDew;   
    /*
    *Format presentation descriptor
    *Définition des format de données pour le DHT22 pour le descripteur 0x2904 conforme à la norme BLE GATT
    */
    static uint8_t presentationTemp[7];
    static uint8_t presentationHumidity[7];

    Characteristic Humidity,Temp,Dew,Heat;
  public :
    DHT22Sensor();
    String getName(){return DHTSensorName;};
    void powerOn();
    void initSensor();
    bool getData();
    void configEnvService(BLEService* pEnvService);
    void printSerialHeader();
    void printSerialData();     
    void setBLEData();   
};
