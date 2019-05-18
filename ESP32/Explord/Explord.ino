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
RTC_DATA_ATTR int readingID = 0;

/* BLE for ESP32 default library on ESP32-arduino framework
  / Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//#include "Characteristic.h"
//#include "Sensor.h"
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
const BLEUUID MultiConnectUUID = BLEUUID("00004862-1000-2000-3000-6578706c6f72");

const BLEUUID SerialStateUUID = BLEUUID("00004871-1000-2000-3000-6578706c6f72");
/*
  BLE Server, Environnmental Sensing Service and Custon Service pointers and for the Sensor singleton
  Declaration des pointeurs pour le serveur BLE, le service des données environnementales, le service BLE personnel et le singleton de la classe Sensor
*/
static BLEServer* pServer = NULL;
static BLEAdvertising* pAdvertising = NULL;
static BLEService* pEnvService = NULL;
static BLEService* pCustomService = NULL;
Sensor* pSensor;

byte SerialOn = 1;
byte BLEOn = 1;
byte LogOn = 0;


/*Define a value for the delay between each reading : Définition de l'intervalle entre deux mesures en secondes*/
unsigned long DelayTime = 0; // for the timer for the sensor readings inside the main loop
uint32_t readDelay = 1;
static BLECharacteristic* pDelay = NULL;

uint8_t setMultiConnect = 0;
static BLECharacteristic* pMultiConnect = NULL;
/*
  Value to store the BLE server connection state
  Valeurs d'états de la connection BLE pour déterminer si il faut emettre les notifications ou non et recommencer a signale le capteur pour le BLE 4.1
*/

unsigned long ReadvertisingTime = 0;
bool deviceConnected = false;
bool oldDeviceConnected = false;

/*
   Callbacks fontion launched by the server when a connection or a disconnection occur
   Fonction definie par la bibliotheque qui est lancée lorsque l'état du serveur BLE change : événement : connexion et deconnexion
*/
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      if (setMultiConnect == 1) { BLEDevice::startAdvertising();} // needed to support multi-connect, that mean more than one client connected to server, must be commented if using BLE 4.0 device
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

/*
 * Callbacks fonction launched when a value of the custom service is modified by a BLE client
 * Fonction définie par la bibliothèque est lancée lorsqu'une valeur a été modifiée par un client BLE
 */
class ClientCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* pData = pCharacteristic->getData();
      if(pCharacteristic == pDelay) {memcpy(&readDelay,pData,4);}
      else if(pCharacteristic == pMultiConnect) {
        memcpy(&setMultiConnect,pData,1);
        pAdvertising->start();
        }
    }
};

static ClientCallbacks* pClientCallbacks = NULL;


class StateCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
      //if (pCharacteristic == _pChar) {}
      //uint8_t* pData = pCharacteristic->getData();
      //if(pCharacteristic == pDelay) {memcpy(&readDelay,pData,4);}
//      if(pCharacteristic == pMultiConnect) {
//        memcpy(&setMultiConnect,pData,1);
//        pAdvertising->start();
//        }
    } 
};

static StateCallbacks* pStateCallbacks = NULL;

class State {
  private :
    byte _On=1;
    int _adress=0;
    BLEUUID _uuid=BLEUUID((uint16_t)0x0000);
    String _Name="";
    static BLECharacteristic* _pChar;
    //StateCallbacks* _pStateCallbacks = NULL;
    void setState(byte state) {
      _On=state;
      if (_adress) {
        EEPROM.write(_adress,(uint8_t) _On);
        EEPROM.commit();
        }
    }
//    class StateCallbacks: virtual public BLECharacteristicCallbacks {
//      public:
//        void onWrite(BLECharacteristic *pCharacteristic) {
//        if (pCharacteristic == _pChar) {}
        //uint8_t* pData = pCharacteristic->getData();
        //if(pCharacteristic == pDelay) {memcpy(&readDelay,pData,4);}
  //      if(pCharacteristic == pMultiConnect) {
  //        memcpy(&setMultiConnect,pData,1);
  //        pAdvertising->start();
  //        }
     //   }
   // };
  public :
    State(int adr,BLEUUID uuid,String Name) : _adress(adr),_uuid(uuid),_Name(Name){};
    void initState() {
      if (_adress) {_On = (byte) EEPROM.read(_adress);}    
    };
    void initBLEState(StateCallbacks* pStateCallbacks) {
      // Create a BLE characteristic 
      _pChar=pCustomService->createCharacteristic(_uuid,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY );
      _pChar->setCallbacks(pStateCallbacks);
      _pChar->setValue((uint8_t*)&_On,1);   
    };
    void switchState(){
      if (_On) { setState(0);}
      else { setState(1);}
      //_pChar->notify();
    }
    byte getState() {
      if (_adress) {_On = (byte) EEPROM.read(_adress);}
      return _On;
    }  
};

static State* pSerialState;


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
  if (incomingOrder == 'D') {
    readDelay=incomingParameter.toInt();
  }
  if (incomingOrder == 'M') {
    if (incomingString.charAt(1) == '0'){setMultiConnect=0;}
    if (incomingString.charAt(1) == '1'){setMultiConnect=1;}
  }
  if (incomingOrder =='H') {
    pSensor->printSerialHeader();
  }
  if (incomingOrder =='N') {
    Serial.println(String(deviceName + pSensor->getName() + "-" + deviceNumber));
  }
  if (incomingOrder =='S') {
/*    if(SerialOn){
      SerialOn=0;
      EEPROM.write(0,0);
      EEPROM.commit();
      }
    else{
      SerialOn=1;
      EEPROM.write(0,1);
      EEPROM.commit();
    }*/
    pSerialState->switchState();
  }
  if (incomingOrder =='B') {
    if(BLEOn){
      BLEOn=0;
      EEPROM.write(1,0);
      EEPROM.commit();
      }
    else{
      BLEOn=1;
      EEPROM.write(1,1);
      EEPROM.commit();
    }
  }
  if (incomingOrder =='L') {
    if(LogOn){
      LogOn=0;
      EEPROM.write(2,0);
      EEPROM.commit();
      }
    else{
      LogOn=1;
      EEPROM.write(2,1);
      EEPROM.commit();
    }
  }
  if (incomingOrder =='R') {
    pLog->readFile("/Explord.csv");
    Serial.println("****************END*******************");
  }
}

void setBLEServer() {
  //Init the BLE Server : Demarrage du serveur BLE
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
  pDelay = pCustomService->createCharacteristic(DelayUUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pClientCallbacks = new ClientCallbacks();
  pDelay->setCallbacks(pClientCallbacks);
  pDelay->setValue((uint8_t*)&readDelay,4);

  //pStateCallbacks = new StateCallbacks();
  // Create a BLE characteristic that enable multiconnect for BLE 4.1 devices : default is false : Caractéristique pour activer les connections multiples pour les client BLE 4.1 minimum
  pMultiConnect = pCustomService->createCharacteristic(MultiConnectUUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pMultiConnect->setCallbacks(pClientCallbacks);
  pMultiConnect->setValue((uint8_t*)&setMultiConnect,1);

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
  //SerialOn= EEPROM.read(0);
  pStateCallbacks = new StateCallbacks();
  pSerialState = new State(1,SerialStateUUID,"SerialState");
  pSerialState->initState();
  if(pSerialState->getState()){Serial.println("Hello I'm there Serial is connected");}
  BLEOn=EEPROM.read(1);
  LogOn=EEPROM.read(2);   
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
  if (BLEOn) {setBLEServer();}  
  if (pSerialState->getState()) {pSensor->printSerialHeader();}
  if (LogOn) {
    pLog = new Log(pSensor);
    pLog->initSD();
  }
  delay(1000); // The sensor need about 1 second to calculate new values : Il faut laisser du temps au capteur pour calculer sa première valeur
}

void loop() {
  /*
   * Read the sensor data according to the delay and send it through BLE and Serial
   */
  if(millis() > DelayTime + (readDelay*1000)) {
    DelayTime=millis();
    if (pSensor->readData()) { //true if new datas are collected by DHT sensor : vrai si des nouvelles données envoyées par le DHT sont disponibles
      if(pSerialState->getState()){pSensor->printSerialData();}  
      if (deviceConnected) { // if a BLE device is connected : si un peripherique BLE est connecté
        //Define new value and notify to connected client : Definition et notification des nouvelles valeurs
        //Serial.println("Sending data through BLE");
        pSensor->setBLEData();
      }
      if (LogOn) {pLog->logSD();}
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
  // disconnecting // give the bluetooth stack the chance to get things ready : si aucun client n'est connecté le capteur retente de proposer des données
  if (!deviceConnected and oldDeviceConnected and (millis()> ReadvertisingTime+ 500 )) {
    ReadvertisingTime=millis();
    //pServer->startAdvertising(); 
    pAdvertising->start(); // restart advertising
    //Serial.println("Restart advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    //Serial.println("Connection to a BLE client done");
    oldDeviceConnected = deviceConnected;
  }
  /*
   * Serial stuff to read the incoming settings and order through USB Serial port
   */
  if (Serial.available()) {
    checkSerial();
  }
}
