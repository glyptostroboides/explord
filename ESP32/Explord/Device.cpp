#include "Device.h"

uint8_t mac_adress[8] = DEVICE_MAC;
unsigned long logTime = 0;

void State::initBLEState(BLEService* pService) {
//      // Create a BLE characteristic 
  pChar=pService->createCharacteristic(uuid,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY );
  setBLEState(); 
};

void State::setBLEState(){};
      
void BoolState::setState(byte istate, bool BLE=false) {
  state=istate;
  Serial.println(String(_Name + " is " + state));      
  if (_adress) {
    EEPROM.write(_adress,(uint8_t) state);
    EEPROM.commit();
    }
  if (pChar and !BLE){ setBLEState();}
};
    
void BoolState::switchState() {
  if (state) { setState(0);
  //Serial.println(String(_Name + " is Off"));
    }
  else { setState(1);
    //Serial.println(String(_Name + " is On"));
    }
  pChar->notify();
};
    
byte BoolState::isOn() {
  if (_adress) {state = (byte) EEPROM.read(_adress);}
  return state;
};  

void BLEState::switchState() { //Can be changed simply by overwriting setBLEState to do nothing : void BLEState::setBLEState (){}; WARNING
  if (state) {setState(0,true);}
  else {setState(1,true);}
};

void BoolState::setBLEState() {
  pChar->setValue((uint8_t*)&state,1);
};

void ValueState::setBLEState() {
  pChar->setValue((uint8_t*)&value,4);
  pChar->notify();
};

void ValueState::storeValue() {
  EEPROM.writeUInt(_adress,value);
  EEPROM.commit();
};

void ValueState::setValue(uint32_t Value) {
  value=Value;
  storeValue();
  setBLEState();
};

void StringState::setBLEState() {
  pChar->setValue((uint8_t*)str,str_size);
  pChar->notify();
};

void StringState::storeString() {
  EEPROM.writeString(str_size,str);
  EEPROM.commit();
};

void StringState::setString (char string[]) {
  strcpy(str,string);
  storeString();
  setBLEState();
};

/*
  BLE Server, Environnmental Sensing Service and Custom Service pointers and for the Sensor singleton
  Declaration des pointeurs pour le serveur BLE, le service des données environnementales, le service BLE personnel et le singleton de la classe Sensor
*/
BLEServer *Device::pServer = NULL;
BLEAdvertising *Device::pAdvertising = NULL;
BLEService *Device::pEnvService = NULL;
BLEService *Device::pCustomService = NULL;

/*
  Value to store the BLE server connection state
  Valeurs d'états de la connection BLE pour déterminer si il faut emettre les notifications ou non et recommencer a signale le capteur pour le BLE 4.1
*/
bool Device::BLEConnected=false;
bool Device::oldBLEConnected=false;
bool Device::isTimerWakeUp=false;

Sensor* Device::pSensor=NULL;
Device::States* Device::pStates=NULL;
Log* Device::pLog=NULL;

/*
BoolState* Device::States::pMultiConnectState = new BoolState(0,MultiConnectStateUUID,"Multiconnect");
BoolState* Device::States::pSerialState = new BoolState(1,SerialStateUUID,"Serial");
BLEState* Device::States::pBLEState = new BLEState(2,BLEStateUUID,"BLE");
BoolState* Device::States::pLogState = new BoolState(3,LogStateUUID,"Log");
BoolState* Device::States::pEcoState = new BoolState(4,EcoStateUUID,"Eco");

ValueState* States::pReadDelay = new ValueState(10,DelayUUID,"Delay");
StringState* States::pLogFilePath = new StringState(20,LogFilePathUUID,"Log File Path",40);*/

BoolState* Device::States::pMultiConnectState = NULL;
BoolState* Device::States::pSerialState = NULL;
BLEState* Device::States::pBLEState = NULL;
BoolState* Device::States::pLogState = NULL;
BoolState* Device::States::pEcoState = NULL;

ValueState* Device::States::pReadDelay = NULL;
StringState* Device::States::pLogFilePath = NULL;


Device::States::States() {
  /*Start the EEPROM memory management and get the module persistant state values*/
  EEPROM.begin(EEPROM_SIZE);
  
  pMultiConnectState = new BoolState(0,MultiConnectStateUUID,"Multiconnect");
  pSerialState = new BoolState(1,SerialStateUUID,"Serial");
  pBLEState = new BLEState(2,BLEStateUUID,"BLE");
  pLogState = new BoolState(3,LogStateUUID,"Log");
  pEcoState = new BoolState(4,EcoStateUUID,"Eco");

  pReadDelay = new ValueState(10,DelayUUID,"Delay");
  pLogFilePath = new StringState(20,LogFilePathUUID,"Log File Path",40);
};

/*
 * Callbacks fonction launched when a value of the custom service is modified by a BLE client
 * Fonction définie par la bibliothèque est lancée lorsqu'une valeur a été modifiée par un client BLE
 */
void Device::States::StateCallbacks::onWrite (BLECharacteristic *pCharacteristic) { //TODO : remove this test by adding a callback for each State !!
  if (pCharacteristic == pMultiConnectState->pChar) {
      pMultiConnectState->switchState();
      pAdvertising->start(); //WARNING TEMPORARY
      }
  if (pCharacteristic == pSerialState->pChar) {pSerialState->switchState();}
  if (pCharacteristic == pLogState->pChar) {
      pLogState->switchState();
      if (pLogState->isOn()){startLog();} //WARNING TEMPORARY
      }
  if (pCharacteristic == pEcoState->pChar) {pEcoState->switchState();}
  if (pCharacteristic == pReadDelay->pChar) {
      uint8_t* pData = pCharacteristic->getData();
      memcpy(&pReadDelay->value,pData,4);
      pReadDelay->storeValue();
      }
};


void Device::States::configBLEService(BLEService* pService) { //TO DO : Can be changed to a loop : for pState in {pMulticonnect, ...} {pState->initBLEState(pService);pState->pCallback;}
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

  // Create a BLE characteristic to old the timespan between two readings : Creation d'une caractéristique contenant l'intervalle entre deux mesures : 4 octets
  pReadDelay->initBLEState(pService);
  pReadDelay->pChar->setCallbacks(&StateCallback);
};


Device::Device() {
  checkTimerWakeUp();
  pStates = new States();
  //pStates->initStates(); //initialize with constructor
  getSensor();
};

void Device::getSensor() {
  pinMode(PPPOWERPIN,OUTPUT);
  digitalWrite(PPPOWERPIN, HIGH); //set high the pin to power the resistor bridge
  delay(10);
  int raw;
  uint8_t plugged_sensor;
  raw = analogRead(PPDETECTPIN);//read the resistor bridge value through adc1
  Serial.println(raw);
  digitalWrite(PPPOWERPIN, LOW);
  //Returned value with a varistor with 14 grade
  if (raw) {
    //Values mesured with a 10kOhm resistor bridge
    if (raw >4000 || raw<1400) {
      Serial.println("No sensor detected with this signature :");
      Serial.println(raw);
      plugged_sensor= 0;
    }
    else if (raw>2800) { //DHT varistor 1
      pSensor = new DHT();
      plugged_sensor= 1;
    }
    else if (raw>2400) { //LOX varistor 2
      pSensor = new LOX();
      plugged_sensor= 2;
    }
    else if (raw>2100) { //MHZ varistor 3
      pSensor = new MHZ();
      plugged_sensor= 3;
    }
    else if (raw>1900) { //DS varistor 4
      pSensor = new DS();
      plugged_sensor= 4;
    }
    else if (raw>1780) { //TSL varistor 5
      pSensor = new TSL();
      plugged_sensor= 5;
    }
    else if (raw>1400) { //BME varistor 6
      pSensor = new BME();
      plugged_sensor= 6;
    }
  mac_adress[3] = plugged_sensor;
  mac_adress[5] = deviceNumber.toInt();
  //Determine the mac adress of the device depending on sensor connected and default device number
  }
};

void Device::startSensor() {
  pSensor->powerOn();
  pSensor->init();
};

void Device::checkTimerWakeUp() {
  /* Get the wake up reason, to detect timer wake up of user wake up (switch off and on)
   * Détermination du mode de réveil : minuteur programmé ou bouton marche arrêt
   */
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER : 
      Serial.println("Wakeup from sleep"); 
      isTimerWakeUp = true;
    default : 
      Serial.println("Wakeup from power on"); 
      isTimerWakeUp = false;
    }
}

void Device::setBLEServer () {
  //Init the BLE Server : Demarrage du serveur BLE fournissant les valeurs mesurées et les paramétres d'état du module
  // Create the BLE Device : Creation du peripherique BLE et definition de son nom qui s'affichera lors du scan : peut contenir une reference unique egalement
  BLEDevice::init((deviceName + pSensor->getName() + "-" + deviceNumber).c_str());

  // Create the BLE Server : Creation du serveur BLE et mise en place de la fonction de callback pour savoir si le serveur est connecté et doit commencer à envoyer des notifications
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(&ServerCallback);

  // Create the BLE Services for the Environnemental Sensing Data and the Custom one: Creation du service pour les données environnementales et du service personnalisé
  pEnvService = pServer->createService(EnvServiceUUID);
  pCustomService = pServer->createService(CustomServiceUUID);
  // Create all the BLE Characteristics according to the sensor plugged into the module : création des caractéristiques BLE en fonction du capteur connecté
  pSensor->configBLEService(pEnvService);

  // Create all the BLE Characteristics according to the EEPROM saved settings : création des caractéristiques BLE pour les paramètres du module stocké en EEPROM
  pStates->configBLEService(pCustomService);

  // Start the services : Demarrage des services sur les données environnementales et du service personnalisé
  pEnvService->start();
  pCustomService->start();

  // Start advertising : Demarrage des notifications pour le client
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(EnvServiceUUID);
  pAdvertising->addServiceUUID(CustomServiceUUID);  
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  pAdvertising->start();
};

void Device::stopBLEServer() {
  pAdvertising->stop();
};

void Device::startAdvertisingBLE() {
  pAdvertising->start();
};

void Device::startLog() {
  logTime=0;
  pLog = new Log(pSensor,pStates->pLogFilePath->str);
  pLog->initSD();
};

void Device::getStates() {
  /* Init the services according to the states of the device
   * Démarrage des services conformément aux valeurs d'états du module
   */
  if (pStates->pLogState->isOn()) {
      pLog = new Log(pSensor,States::pLogFilePath->str);
      pLog->initSD();
      }
  if (pStates->pEcoState->isOn()) {
      if(isTimerWakeUp) {
        States::pEcoState->switchState();
        }
      else {doReadAndSleep();}
      }
  if (pStates->pBLEState->isOn()) {
      esp_base_mac_addr_set(mac_adress);     
      setBLEServer();
      }  
      
  if (pStates->pSerialState->isOn()) {pSensor->printSerialHeader();}
};

void Device::getSerial() {
  String incomingString = Serial.readStringUntil('\r');
  char incomingOrder = incomingString.charAt(0);
  String incomingParameter = incomingString.substring(1);
  switch (incomingOrder) {
    case 'D' :
      pStates->pReadDelay->setValue(incomingParameter.toInt());
      break;
    case 'H' :
      pSensor->printSerialHeader();
      break;
    case 'N' :
      Serial.println(String(deviceName + pSensor->getName() + "-" + deviceNumber));  
      break;
    case 'M' :
      pStates->pMultiConnectState->switchState();
      break;
    case 'S' :
      pStates->pSerialState->switchState();
      break;
    case 'B' :  
      if (pStates->pBLEState->isOn()) {
        stopBLEServer();
      }
      else {setBLEServer();};
      pStates->pBLEState->switchState();
      break;
    case 'E' :
      pStates->pEcoState->switchState();
      break;
    case 'L' :
      if (incomingString.charAt(1)==' ') {
        if (incomingString.charAt(2)=='/'){
          incomingString= incomingString.substring(2) + ".csv";
          }
        else {
          incomingString= "/" + incomingString.substring(2) + ".csv";
          }
        incomingString.toCharArray(pStates->pLogFilePath->str,44);
        pStates->pLogFilePath->storeString();
        }
      pStates->pLogState->switchState();
      if (pStates->pLogState->isOn()){
         startLog();
        }
      break;
    case 'R' :
      pLog->readFile();
      Serial.println("****************END*******************");
      break; 
  }
};

bool Device::doRead() {
  if (pSensor->readData()){ //true if new datas are collected by sensor : vrai si des nouvelles données envoyées par le capteur sont disponibles    if (Serial.available()) {checkSerial();}
    if (pStates->pSerialState->isOn()){pSensor->printSerialData(&logTime);} // if Serial is on : print data to serial USB
    if (BLEConnected) {pSensor->setBLEData();} // if a BLE device is connected
    if (pStates->pLogState->isOn()) {pLog->logSD(&logTime);} // if Log on log to the current log file
    if (pStates->pEcoState->isOn()) {
      logTime+=pStates->pReadDelay->value;
      doSleep();
      }
    return true;
  }
  else {return false;}
};

void Device::doReadAndSleep() {
  //delay(1000);
  if (doRead()) { doSleep();}
};

void Device::doSleep() {
  pSensor->powerOff();
  unsigned long TimerDelay = (pStates->pReadDelay->value *1000000);
  if (TimerDelay > micros()) {TimerDelay-=micros();}
  esp_sleep_enable_timer_wakeup(TimerDelay);
  Serial.println("Going to sleep now for : ");
  Serial.println(TimerDelay);
  Serial.flush(); 
  esp_deep_sleep_start();
};
