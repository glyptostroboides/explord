#include "Device.h"

void State::setBLEState() {
      pChar->setValue((uint8_t*)&state,1);
    };
    
void State::initBLEState(BLEService* pService) {
//      // Create a BLE characteristic 
      pChar=pService->createCharacteristic(uuid,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY );
      setBLEState(); 
      };
      
void State::setState(byte istate, bool BLE=false) {
      state=istate;
      Serial.println(String(_Name + " is " + state));      
      if (_adress) {
        EEPROM.write(_adress,(uint8_t) state);
        EEPROM.commit();
        }
      if (pChar and !BLE){ setBLEState();}
    };
    
void State::switchState(){
      if (state) { setState(0);
      //Serial.println(String(_Name + " is Off"));
        }
      else { setState(1);
        //Serial.println(String(_Name + " is On"));
        }
      pChar->notify();
    };
    
byte State::isOn() {
      if (_adress) {state = (byte) EEPROM.read(_adress);}
      return state;
    };  

void BLEState::switchState () {
    if (state) {
          //pChar=NULL;
       setState(0,true);}
    else {
       setState(1,true);
    }
};

void Device::StateCallbacks::onWrite (BLECharacteristic *pCharacteristic) {
   if (pCharacteristic == pMultiConnectState->pChar) {
          pMultiConnectState->switchState();
          //pAdvertising->start(); WARNING TEMPORARY
        }
   if (pCharacteristic == pSerialState->pChar) {pSerialState->switchState();}
   if (pCharacteristic == pLogState->pChar) {
        pLogState->switchState();
        //if (pLogState->isOn()){ startLog(); } WARNING TEMPORARY
        }
   if (pCharacteristic == pEcoState->pChar) {pEcoState->switchState();}
};

void Device::initStates () {
  /*Start the EEPROM memory management and get the module persistant state values*/
  EEPROM.begin(EEPROM_SIZE);
  pMultiConnectState = new State(0,MultiConnectStateUUID,"Multiconnect");
  pSerialState = new State(1,SerialStateUUID,"Serial");
  pBLEState = new BLEState(2,BLEStateUUID,"BLE");
  pLogState = new State(3,LogStateUUID,"Log");
  pEcoState = new State(4,EcoStateUUID,"Eco");
};

void Device::configBLEService(BLEService* pService) {
  //pStateCallbacks = new StateCallbacks();
  // Create a BLE characteristic that enable multiconnect for BLE 4.1 devices : Caractéristique pour activer les connections multiples pour les client BLE 4.1 minimum
  pMultiConnectState->initBLEState(pService);
  pMultiConnectState->pChar->setCallbacks(&StateCallback);
  
  // Create a BLE characteristic that enable Serial : Caractéristique pour activer l'envoie des données par le port série
  pSerialState->initBLEState(pService);
  pSerialState->pChar->setCallbacks(&StateCallback);

  // Create a BLE characteristic that enable log to a file on onboard SD Card : Caractéristique pour activer l'enregistrement des mesures sur la carte SD
  pLogState->initBLEState(pService);
  pLogState->pChar->setCallbacks(&StateCallback);

  // Create a BLE characteristic to enable the eco mode that turn off the device and sensor between readings
  pEcoState->initBLEState(pService);
  pEcoState->pChar->setCallbacks(&StateCallback);
};
