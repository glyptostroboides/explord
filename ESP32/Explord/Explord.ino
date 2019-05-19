/*
  This program send environmental data through BLE with an ESP32.
  The UUID used are the one from the BLE GATT specifications : https://www.bluetooth.com/specifications/gatt
*/

/*
   This part is not specific to any sensor. BLE Server and Services that are used by all sensorts
*/
/*
 * 
 */
const String deviceName = "Explord-";
const String deviceNumber = "01"; //added to the Sensor specific device name


unsigned long LedTime=0;
unsigned long BlinkTime=30;
bool LedOn=false;
/*Define the pin for builtin LEDPin : Definition de la broche pour la  intégrée 22 et non 21 comme l'indique le peu de documentation*/
const int LEDPin = 22;

/*Define the pin for the Plug and Play feature of the module : définition des connecteurs qui servent à déterminer le capteur connecté au démarrage du module*/
const int PlugPin1 = 16;
const int PlugPin2 = 17;
const int PlugPin3 = 18;
/*Select automatically the sensor used to init the sensor class
  Possible values are :
  1 : DHT_22 : humidity and temperature sensor
  2 : LOX02 : dioxygen rate sensor (also temperature, pressure and O2 partial pressure)
  3 : MHZ16 : carbon dioxyd rate sensor
*/

/*Define the module state stored in the EEPROM persistant memory: communication (Serial, BLE, ...)
 * Definition de l'état du module : communication et autres
 */
#include "EEPROM.h"
const int EEPROM_SIZE = 64;

/*Define the logging capabilities to the SD Card*/
#include "Log.h"

Log* pLog;
 
// Save reading number on RTC memory, used when the deep sleep is launched
//RTC_DATA_ATTR int readingID = 0;

/* BLE for ESP32 default library on ESP32-arduino framework
  / Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "Drivers.h"

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
const BLEUUID DelayUUID = BLEUUID("00004861-1000-2000-3000-6578706c6f72");

const BLEUUID MultiConnectStateUUID = BLEUUID("00004870-1000-2000-3000-6578706c6f72");
const BLEUUID SerialStateUUID = BLEUUID("00004871-1000-2000-3000-6578706c6f72");
const BLEUUID LogStateUUID = BLEUUID("00004872-1000-2000-3000-6578706c6f72");
const BLEUUID BLEStateUUID = BLEUUID("00004873-1000-2000-3000-6578706c6f72");

/*
  BLE Server, Environnmental Sensing Service and Custon Service pointers and for the Sensor singleton
  Declaration des pointeurs pour le serveur BLE, le service des données environnementales, le service BLE personnel et le singleton de la classe Sensor
*/
static BLEServer* pServer = NULL;
static BLEAdvertising* pAdvertising = NULL;
static BLEService* pEnvService = NULL;
static BLEService* pCustomService = NULL;
Sensor* pSensor;

/*Define a value for the delay between each reading : Définition de l'intervalle entre deux mesures en secondes*/
unsigned long DelayTime = 0; // for the timer for the sensor readings inside the main loop
uint32_t readDelay = 1;
static BLECharacteristic* pDelay = NULL;

/*
  Value to store the BLE server connection state
  Valeurs d'états de la connection BLE pour déterminer si il faut emettre les notifications ou non et recommencer a signale le capteur pour le BLE 4.1
*/

unsigned long ReadvertisingTime = 0;
bool BLEConnected = false;
bool oldBLEConnected = false;


/*
 * Callbacks fonction launched when a value of the custom service is modified by a BLE client
 * Fonction définie par la bibliothèque est lancée lorsqu'une valeur a été modifiée par un client BLE
 */
class ClientCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* pData = pCharacteristic->getData();
      if(pCharacteristic == pDelay) {memcpy(&readDelay,pData,4);}
    }
};

static ClientCallbacks* pClientCallbacks = NULL;

class State {
  private :
    int _adress=0;
    //BLEUUID _uuid=BLEUUID((uint16_t)0x0000);
    String _Name="";
    //StateCallbacks* _pStateCallbacks = NULL;
    void setState(byte istate) {
      state=istate;
      if (_adress) {
        EEPROM.write(_adress,(uint8_t) state);
        EEPROM.commit();
        }
      if (pChar){ setBLEState();}
    };
    void setBLEState() {
      pChar->setValue((uint8_t*)&state,1);
    };
  public :
    State(int adr,BLEUUID id,String Name) : _adress(adr),uuid(id),_Name(Name) {
      state = (byte) EEPROM.read(_adress);
      };
    byte state=1;
    BLEUUID uuid=BLEUUID((uint16_t)0x0000);
    BLECharacteristic* pChar;
    void initBLEState() {
//      // Create a BLE characteristic 
      pChar=pCustomService->createCharacteristic(uuid,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY );
      setBLEState(); 
      };
    void switchState(){
      if (state) { setState(0);}
      else { setState(1);}
      pChar->notify();
    };
    byte isOn() {
      if (_adress) {state = (byte) EEPROM.read(_adress);}
      return state;
    };  
};

static State* pMultiConnectState;
static State* pSerialState;
static State* pBLEState;
static State* pLogState;
String CurrentLogFile="/explord.csv";

class StateCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
      if (pCharacteristic == pMultiConnectState->pChar) {
        pMultiConnectState->switchState();
        pAdvertising->start();
        }
      if (pCharacteristic == pSerialState->pChar) {pSerialState->switchState();}
      if (pCharacteristic == pLogState->pChar) {
        pLogState->switchState();
        if (pLogState->isOn()){
        pLog = new Log(pSensor,"/test3.csv");
        pLog->initSD();
        }
      }
    } 
};

static StateCallbacks* pStateCallbacks = NULL;

/*
   Callbacks fontion launched by the server when a connection or a disconnection occur
   Fonction definie par la bibliotheque qui est lancée lorsque l'état du serveur BLE change : événement : connexion et deconnexion
*/
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      BLEConnected = true;
      if (pMultiConnectState->isOn()) { BLEDevice::startAdvertising();} // needed to support multi-connect, that mean more than one client connected to server, must be commented if using BLE 4.0 device
    };

    void onDisconnect(BLEServer* pServer) {
      BLEConnected = false;
    }
};


uint8_t getPluggedSensor() { // Get the sensor id by checking the code pin which are high : detection du capteur connecté en identifiant les connecteurs HIGH
  //For testing purpose in order to use this pin as power pin
  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);
  delay(10);
  pinMode(PlugPin1, INPUT);
  pinMode(PlugPin2, INPUT);
  pinMode(PlugPin3, INPUT);
  int Pin1 = digitalRead(PlugPin1);
  int Pin2 = digitalRead(PlugPin2);
  int Pin3 = digitalRead(PlugPin3);
  uint8_t sensor_id = Pin1 + 2*Pin2 + 4*Pin3;
  digitalWrite(23,LOW);
  if (sensor_id==0) {Serial.println("No sensor connected");}
  return sensor_id;
}

void checkSerial() {
  String incomingString = Serial.readStringUntil('\r');
  char incomingOrder = incomingString.charAt(0);
  String incomingParameter = incomingString.substring(1);
  switch (incomingOrder) {
    case 'D' :
      readDelay=incomingParameter.toInt();
      break;
    case 'H' :
      pSensor->printSerialHeader();
      break;
    case 'N' :
      Serial.println(String(deviceName + pSensor->getName() + "-" + deviceNumber));  
      break;
    case 'M' :
      pMultiConnectState->switchState();
      break;
    case 'S' :
      pSerialState->switchState();
      break;
    case 'B' :
      pBLEState->switchState();
      break;
    case 'L' :
      //if (incomingString.charAt(1)==' ') {CurrentLogFile=incomingString.substring(2);}
      pLogState->switchState();
      if (pLogState->isOn()){
         pLog = new Log(pSensor,"/test3.csv");
         pLog->initSD();
        }
      break;
    case 'R' :
      pLog->readFile();
      Serial.println("****************END*******************");
      break; 
  }
}

void setBLEServer() {
  //Init the BLE Server : Demarrage du serveur BLE fournissant les valeurs mesurées et les paramétres d'état du module
  // Create the BLE Device : Creation du peripherique BLE et definition de son nom qui s'affichera lors du scan : peut contenir une reference unique egalement
  BLEDevice::init((deviceName + pSensor->getName() + "-" + deviceNumber).c_str());

  // Create the BLE Server : Creation du serveur BLE et mise en place de la fonction de callback pour savoir si le serveur est connecté et doit commencer à envoyer des notifications
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create the BLE Services for the Environnemental Sensing Data and the Custom one: Creation du service pour les données environnementales et du service personnalisé
  pEnvService = pServer->createService(EnvServiceUUID);
  pCustomService = pServer->createService(CustomServiceUUID);
  // Create all the BLE Characteristics according to the sensor plugged into the module : création des caractéristiques BLE en fonction du capteur connecté
  pSensor->configBLEService(pEnvService);

  // Create a BLE characteristic to old the timespan between two readings : Creation d'une caractéristique contenant l'intervalle entre deux mesures : 4 octets
  pClientCallbacks = new ClientCallbacks();
  pDelay = pCustomService->createCharacteristic(DelayUUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pDelay->setCallbacks(pClientCallbacks);
  pDelay->setValue((uint8_t*)&readDelay,4);

  pStateCallbacks = new StateCallbacks();
  // Create a BLE characteristic that enable multiconnect for BLE 4.1 devices : Caractéristique pour activer les connections multiples pour les client BLE 4.1 minimum
  pMultiConnectState->initBLEState();
  pMultiConnectState->pChar->setCallbacks(pStateCallbacks);
  
  // Create a BLE characteristic that enable Serial : Caractéristique pour activer l'envoie des données par le port série
  pSerialState->initBLEState();
  pSerialState->pChar->setCallbacks(pStateCallbacks);

  // Create a BLE characteristic that enable log to a file on onboard SD Card : Caractéristique pour activer l'enregistrement des mesures sur la carte SD
  pLogState->initBLEState();
  pLogState->pChar->setCallbacks(pStateCallbacks);

  // Start the services : Demarrage des services sur les données environnementales et du service personnalisé
  pEnvService->start();
  pCustomService->start();

  // Start advertising : Demarrage des notifications pour le client
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(EnvServiceUUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  pAdvertising->start();
}

void setup() {
  /*Set the internal led as an output for blinking purpose*/
  pinMode(LEDPin, OUTPUT);
  /*Start the EEPROM memory management and get the module persistant state values*/
  EEPROM.begin(EEPROM_SIZE);
  
  pStateCallbacks = new StateCallbacks();
  
  pMultiConnectState = new State(0,MultiConnectStateUUID,"Multiconnect");
  pSerialState = new State(1,SerialStateUUID,"Serial");
  pBLEState = new State(2,BLEStateUUID,"BLE");
  pLogState = new State(3,LogStateUUID,"Log");
  
  /* Init the serial connection through USB
     Demarrage de la connection serie a travers le port USB
  */
  Serial.begin(115200);
  
  uint8_t plugged_sensor = getPluggedSensor();
  switch(plugged_sensor) {
    case 1: 
      pSensor = new DHT();
      break;
    case 2:
      pSensor = new LOX();
      break;
    case 3:
      pSensor = new MHZ();
      break;
    case 4:
      pSensor = new DS();
      break;
    case 5:
      pSensor = new TSL();
      break;
  }
  
  pSensor->init();
  pSensor->powerOn();
  
  if (pBLEState->isOn()) {setBLEServer();}  
  if (pSerialState->isOn()) {pSensor->printSerialHeader();}
  if (pLogState->isOn()) {
    pLog = new Log(pSensor,"/test3.csv");
    pLog->initSD();
  }
  delay(2000); // The sensor need about 1 second to calculate new values : Il faut laisser du temps au capteur pour calculer sa première valeur
}

void loop() {
  /*
   * Read the sensor data according to the delay and send it through BLE and Serial
   */
  if(millis() > DelayTime + (readDelay*1000)) {
    DelayTime=millis();
    if (pSensor->readData()) { //true if new datas are collected by DHT sensor : vrai si des nouvelles données envoyées par le DHT sont disponibles
      if(pSerialState->isOn()){pSensor->printSerialData();}  
      if (BLEConnected) { // if a BLE device is connected : si un peripherique BLE est connecté
        pSensor->setBLEData();
      }
      if (pLogState->isOn()) {pLog->logSD();}
      digitalWrite(LEDPin, HIGH);
      LedTime=millis();
      LedOn=true;
    }
  }
  /*Turn of the led lighted on after last reading*/
  if(LedOn and (millis() > LedTime + BlinkTime)) {
    LedTime=millis();
    digitalWrite(LEDPin, LOW);
    LedOn=false;
  }
  /*
   * BLE Connecting and Disconnecting stuff
   */
  // disconnecting // give the bluetooth stack the chance to get things ready : si aucun client n'est connecté le capteur retente de proposer des données après 500ms
  if (!BLEConnected and oldBLEConnected and (millis()> ReadvertisingTime+ 500 )) {
    ReadvertisingTime=millis();
    //pServer->startAdvertising(); 
    pAdvertising->start(); // restart advertising
    oldBLEConnected = BLEConnected;
  }
  // connecting
  if (BLEConnected && !oldBLEConnected) { oldBLEConnected = BLEConnected; }// Connection to a BLE client done : connection à un client BLE effectuée
  /*
   * Serial stuff to read the incoming settings and order through USB Serial port
   */
  if (Serial.available()) {
    checkSerial();
  }
}
