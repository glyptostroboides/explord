#pragma once

#include "Configuration.h"
#include "Drivers.h" //Manage the sensor
#include "Log.h" //Manage the storage of values inside SD

#include "esp_system.h" //Used to modify mac adress of the device

#include <Arduino.h>

/* BLE for ESP32 default library on ESP32-arduino framework
  / Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "EEPROM.h" //Used to store device state configurations inside a persistant memory

/*
   This part is not specific to any sensor. BLE Server and Services that are used by all sensors
*/
const String deviceName = DEVICE_NAME;
const String deviceNumber = DEVICE_NUMBER; //added to the Sensor specific device name

/*To change the mac adress when the sensor is changed in order to avoid a bug in the Web Bluetooth API because characteristic are changing
 */
 //Original esp ttgo t1 mac adress are of type : 80:7D:3A:E4:
extern uint8_t mac_adress[8];

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

const BLEUUID MultiConnectStateUUID = BLEUUID("00004870-1000-2000-3000-6578706c6f72");
const BLEUUID SerialStateUUID = BLEUUID("00004871-1000-2000-3000-6578706c6f72");
const BLEUUID LogStateUUID = BLEUUID("00004872-1000-2000-3000-6578706c6f72");
const BLEUUID BLEStateUUID = BLEUUID("00004873-1000-2000-3000-6578706c6f72");
const BLEUUID EcoStateUUID = BLEUUID("00004874-1000-2000-3000-6578706c6f72");

const BLEUUID DelayUUID = BLEUUID("00004861-1000-2000-3000-6578706c6f72");
const BLEUUID LogFilePathUUID = BLEUUID("00004862-1000-2000-3000-6578706c6f72");

/* Define the time in second since log started : Definition du temps depuis le début de l'enregistrement */

//RTC_DATA_ATTR char CurrentLogFile[20]=DEFAULT_LOG_FILE;//log file name, saved for deep sleep
extern RTC_DATA_ATTR unsigned long logTime; //Store the time between the begin of log to save or display it : temps depuis le début de l'acquisition à afficher ou à transmettre
//RTC_DATA_ATTR uint32_t readDelay = READ_DELAY; 

class State {
  public :
    State(int adr,BLEUUID id,String Name) : _adress(adr),uuid(id),_Name(Name) {};
    //byte state=1;
    int _adress=0;
    String _Name="";
    BLEUUID uuid=BLEUUID((uint16_t)0x0000);
    BLECharacteristic* pChar;
    void initBLEState(BLEService* pService);
};

class BoolState : public State {
  private :
    void setBLEState();
  public :
    BoolState(int adr,BLEUUID id,String Name) : State(adr,id,Name) {
      state = (byte) EEPROM.read(_adress);
      };
    byte state=1;
    void setState(byte istate, bool BLE);
    void switchState();
    byte isOn();  
};

class BLEState : public BoolState {
  public:
    BLEState(int adr,BLEUUID id,String Name): BoolState(adr,id,Name){};
    void switchState ();
};

class ValueState : public State { // class for the read delay
  private:
    void setBLEState();
  public:
    ValueState(int adr,BLEUUID id,String Name) : State(adr,id,Name) {
      value = (uint32_t) EEPROM.readUInt(_adress);
      };
    uint32_t value=1;
    void storeValue();
    void setValue(uint32_t Value);
};

class StringState : public State { //class to store the file path for logging
  private:
    void setBLEState();
  public:
    StringState(int adr,BLEUUID id,String Name, uint8_t String_size) : State(adr,id,Name),str_size(String_size) {
      EEPROM.readString(_adress,str,str_size);      
      };
    uint8_t str_size;
    char str[44];
    void storeString();
    void setString(char string[]);
};


class Device {
  private : 
    static Sensor *pSensor; /*Define the instance pointer of the sensor, the log file, ...*/
    static Log *pLog;
    /*
    BLE Server, Environnmental Sensing Service and Custom Service pointers and for the Sensor singleton
    Declaration des pointeurs pour le serveur BLE, le service des données environnementales, le service BLE personnel et le singleton de la classe Sensor
    */
    static BLEServer *pServer;
    static BLEAdvertising *pAdvertising;
    static BLEService *pEnvService, *pCustomService;
    /*
    * Callbacks fonction launched when a value of the custom service is modified by a BLE client
    * Fonction définie par la bibliothèque est lancée lorsqu'une valeur a été modifiée par un client BLE
    */
    class ServerCallbacks: public BLEServerCallbacks {
      void onConnect(BLEServer* pServer) {
      BLEConnected = true;
      restartAdvertisingBLE();//needed to support multi-connect(not done if multiconnect is not setted), that mean more than one client connected to server
      }; 
      void onDisconnect(BLEServer* pServer) {
      BLEConnected = false;
      };
    } ServerCallback; 
  public:
    Device();
    class States;
    /*
    Value to store the BLE server connection state
    Valeurs d'états de la connection BLE pour déterminer si il faut emettre les notifications ou non et recommencer a signale le capteur pour le BLE 4.1
    */
    static bool BLEConnected;
    static bool oldBLEConnected;
    //static bool isTimerWakeUp;
    static States* pStates; 
    static bool isTimerWakeUp();
    void initSettings();
    void getSensor();
    void startSensor();
    void setBLEServer();
    static void startAdvertisingBLE();
    static void restartAdvertisingBLE();
    void stopBLEServer();
    static void startLog();
    void getSerial();
    bool doRead();
    static void doSleep();
    void doReadAndSleep();
};

class Device::States {
  private : 
    class StateCallbacks: public BLECharacteristicCallbacks {
      void onWrite(BLECharacteristic *pCharacteristic);
    } StateCallback;
  public :
    States();
    static BoolState *pMultiConnectState, *pSerialState, *pLogState, *pEcoState;
    static BLEState* pBLEState;
    static ValueState* pReadDelay; //Define a value for the delay between each reading : Définition de l'intervalle entre deux mesures en secondes
    static StringState* pLogFilePath;
    //void initStates();
    void configBLEService(BLEService* pService);
};
