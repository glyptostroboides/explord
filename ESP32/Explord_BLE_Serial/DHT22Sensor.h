/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "BLE_Char.h"

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
    TempAndHumidity DHTData;
    /*Store the integer from DHT*/
    uint16_t dHumidity;
    int16_t dTemp; 
    int16_t dHeat;
    int16_t dDew;
    /*Store the values sended by DHT as strings*/
    String sTemp;
    String sHumidity;
    String sHeat;
    String sDew;
    /*
    *BLE characteristics pointers for the DHT
    *Declaration des pointeurs pour les caracteristiques BLE 
    */
    BLECharacteristic* pTemp = NULL;
    BLECharacteristic* pHumidity = NULL;
    BLECharacteristic* pHeat = NULL;
    BLECharacteristic* pDew = NULL;
    /* 
    *  Define the UUID for the  characteritic used by the DHT Sensor
    *  Definitions des identifiants pour le DHT pour le service "donnees environnementales" conforme aux definitions de la norme BLE
    */
    const BLEUUID TEMP_UUID = BLEUUID((uint16_t)0x2A6E); // 0x2A6E is the characteristic for Temperature from ENV : en degres celsius correspond a un : sint16, Decimal,-2, soit 2 ouchar4 char
    const BLEUUID HUMIDITY_UUID = BLEUUID((uint16_t)0x2A6F); // 0x2A6F : relative humidity in % correspond a un : uint16 ,Decimal, -1, soit 2 char 
    const BLEUUID DEW_UUID = BLEUUID((uint16_t)0x2A7B); // Dew Point in Celsius degrees with two decimals int
    const BLEUUID HEAT_UUID = BLEUUID((uint16_t)0x2A7A); // Heat Index in Celsius degrees
    /*
    *Format presentation descriptor
    *Définition des format de données pour le DHT22 pour le descripteur 0x2904 conforme à la norme BLE GATT
    */
    static uint8_t presentationTemp[7];
    static uint8_t presentationHumidity[7];
    static uint8_t presentationHeat[7];
    static uint8_t presentationDew[7];

    //BLEChar Humidity;
    //BLEChar Temp;
    //BLEChar Heat;
    //BLEChar Dew;
  public :
    String getName(){return DHTSensorName;};
    void powerOn();
    void initSensor();
    bool getData();
    void configEnvService(BLEService* pEnvService);
    void printSerialHeader();
    void printSerialData();     
    void setBLEData();   
};
