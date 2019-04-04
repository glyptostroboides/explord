
#include <Arduino.h>
/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


 class BLEChar {
    private:
      BLEService* _pService;
      BLECharacteristic* _pChar;
      const BLEUUID _UUID;
      String _CharName = "";
      uint8_t* _presentation = NULL;
      uint8_t* _value;
    public:
      //BLEChar(BLEService* pService,BLEUUID UUID);
      BLEChar(BLEUUID uuid,String charname,uint8_t* pres,uint8_t* value): _UUID(uuid),_CharName(charname),_presentation(pres),_value(value){};
      bool initCharacteristic(BLEService* pService);
      bool setCharacteristic(); 
 };
