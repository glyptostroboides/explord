#pragma once

#include "Configuration.h"
#include <Arduino.h>
/* BLE for ESP32 default library on ESP32-arduino framework
  / Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "EEPROM.h" //Used to store device state configurations inside a persistant memory

/*Define the module state stored in the EEPROM persistant memory: communication (Serial, BLE, ...)
 * Definition de l'état du module : communication et autres
 */
const int EEPROM_SIZE = 64;

/*
 * Define the UUID for the BLE GATT environnmental sensing service used by all sensors
 * Definition de l'identifiant unique pour le service des capteurs environnementaux
 */

const BLEUUID EnvServiceUUID = BLEUUID((uint16_t)0x181A); // 0x181A is the service for Environnemental Sensing : service pour les capteurs environnementaux

/*
 * Define the UUID for the custom BLE Service called Explord service used by all sensors
 * Definition de l'identifiant unique pour le service BLE personnel utilisé par tous les capteurs et des identifiants pour ses valeurs
 */
const BLEUUID CustomServiceUUID = BLEUUID("00004860-1000-2000-3000-6578706c6f72"); //like all custom characteristics start with 0000486*
//const BLEUUID DelayUUID = BLEUUID("00004861-1000-2000-3000-6578706c6f72");

const BLEUUID MultiConnectStateUUID = BLEUUID("00004870-1000-2000-3000-6578706c6f72");
const BLEUUID SerialStateUUID = BLEUUID("00004871-1000-2000-3000-6578706c6f72");
const BLEUUID LogStateUUID = BLEUUID("00004872-1000-2000-3000-6578706c6f72");
const BLEUUID BLEStateUUID = BLEUUID("00004873-1000-2000-3000-6578706c6f72");
const BLEUUID EcoStateUUID = BLEUUID("00004874-1000-2000-3000-6578706c6f72");


class State {
  private :
    int _adress=0;
    //BLEUUID _uuid=BLEUUID((uint16_t)0x0000);
    String _Name="";
    //StateCallbacks* _pStateCallbacks = NULL;
    void setBLEState();
  public :
    State(int adr,BLEUUID id,String Name) : _adress(adr),uuid(id),_Name(Name) {
      state = (byte) EEPROM.read(_adress);
      };
    byte state=1;
    BLEUUID uuid=BLEUUID((uint16_t)0x0000);
    BLECharacteristic* pChar;
    void initBLEState(BLEService* pService);
    void setState(byte istate, bool BLE);
    void switchState();
    byte isOn();  
};

class BLEState : public State {
  public:
  BLEState(int adr,BLEUUID id,String Name):State(adr,id,Name){}
  void switchState ();
};

class Device {
  private : 
    static State* pMultiConnectState;
    static State* pSerialState;
    static BLEState* pBLEState;
    static State* pLogState;
    static State* pEcoState;
    class StateCallbacks: public BLECharacteristicCallbacks {
      void onWrite(BLECharacteristic *pCharacteristic);
    } StateCallback;
  public:
    //States(){}
    void initStates();
    void configBLEService(BLEService* pService);
};
