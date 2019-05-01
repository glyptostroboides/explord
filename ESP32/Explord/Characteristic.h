#pragma once

#include <Arduino.h>
/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


 class Characteristic {
    private:
      /*
       * Define the BLE service pointer for the characteristic
       * Definition du pointeur du service auquel est rattache la caracteristique
       */
      BLEService* _pService;
      /*
        * BLE characteristic pointer for the sensor
        * Declaration d'un pointeur pour la caracteristique BLE 
      */
      BLECharacteristic* _pChar;
      /* 
        * Define the UUID for the  characteritic used by the LOX02 Sensor
        * Definitions des identifiants pour le capteur pour le service "donnees environnementales" conforme aux definitions de la norme BLE
        * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml

      */
      BLEUUID _UUID;
      /*
        * Define the name of the characteristics as strings
        * Definition du nom de la caractéristique      
     */
      String _Name = "";
      /*
       * Define a presentation format for the characteristic : an array of 7 bytes defined by the BLE standards
       * Définition du format de présentation des données dans un tableau de 7 octets defini par la norme BLE
       */
      uint8_t* _presentation=NULL;
      /*/
       * Valeur qui peut être de différente taille (int16, uint32, ...)stockée dans un tableau de 4 octets
       */
      uint8_t _value[4]={NULL};
      /*
       * Taille de la valeur pointée par le pointeur ci dessus
       */
      size_t _value_size;
      /*
       * Valeur au format d'une chaine de caractère
       */
      String _svalue;
    public:
      //Class constructor : Constructeur de la classe
      //BLEChar(BLEUUID uuid,String charname,uint8_t* pres,uint8_t* value, size_t value_size): _UUID(uuid),_CharName(charname),_presentation(pres),_value(value), _value_size(value_size){};
      Characteristic(BLEUUID uuid,String cname,uint8_t* pres, size_t value_size): _UUID(uuid),_Name(cname),_presentation(pres), _value_size(value_size){};
      String getName();
      String getSValue();
      void setValue(uint8_t*, size_t);
      void setSValue(String);
      bool initBLECharacteristic(BLEService*);
      bool setBLECharacteristic();
 };
// #endif
