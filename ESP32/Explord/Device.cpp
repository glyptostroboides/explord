#include "Device.h"

uint8_t mac_adress[8] = DEVICE_MAC;
RTC_DATA_ATTR unsigned long logTime = 0;
Preferences Settings;

uint8_t presBool[] = {
  0x01, // Format = 1 = "boolean"
  0x00, // Exponent = 0
  0x27, // Unit = 0x2700 = "unitless" (low byte)
  0x00, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

uint8_t presDelay[] = {
  0x01, // Format = 8 = "unsigned 32-bit integer"
  0x00, // Exponent = 0
  0x27, // Unit = 0x2703 = "time(second)" (low byte)
  0x03, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

uint8_t presLogFilePath[] = {
  0x01, // Format = E = "signed 16-bit integer"
  0x00, // Exponent = 0
  0x27, // Unit = 0x2700 = "unitless" (low byte)
  0x00, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

void State::initBLEState(BLEService* pService) { //Could be easily merged to Characteristic class with a common base class
//      // Create a BLE characteristic 
  pChar=pService->createCharacteristic(uuid,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY );
   // Create a BLE Descriptor with BLE2902 (which manage the Notify settings)
  pChar->addDescriptor(new BLE2902());
  // Define a Descriptor for the name of the characteristic as a string: Definition des descripteurs contenant le nom de la valeur présentée par le serveur
  BLEDescriptor *nameDescriptor = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description : pour indiquer le nom de la valeur mesurée
  pChar->addDescriptor(nameDescriptor);
  nameDescriptor->setValue(_Name.c_str());
  //Define the presentation format for each characteristic ( Characteristic Presentation Format) : Définition des descripteurs contenant les informations sur la presentation des valeurs mesurées  if (_presentation != NULL) {
  BLEDescriptor *presentationDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pChar->addDescriptor(presentationDescriptor);
  presentationDescriptor->setValue(presentation, 7);
};
      
void BoolState::setState(bool istate, bool BLE=false) {
  state=istate;
  Serial.println(String(_Name + " is " + state));      
  Settings.putBool(_Name.c_str(),state);
  //if (pChar and !BLE){ setBLEState();}
  if (!BLE) {setBLEState();} // needed to avoid a bug with turning off BLE through BLE ! this is made impossible
};
    
void BoolState::switchState() {
  if (state) { setState(false);
  //Serial.println(String(_Name + " is Off"));
    }
  else { setState(true);
    //Serial.println(String(_Name + " is On"));
    }
};
    
bool BoolState::isOn() {
  //if (_adress) {state = (byte) EEPROM.read(_adress);}
  return Settings.getBool(_Name.c_str());
};  

void BLEState::switchState() { //Can be changed simply by overwriting setBLEState to do nothing : void BLEState::setBLEState (){}; WARNING
  if (state) {setState(false,true);}
  else {setState(true,true);}
};

void BoolState::setBLEState(bool notification) {
  pChar->setValue((uint8_t*)&state,1);
  if(notification){pChar->notify();}
};

void ValueState::setBLEState(bool notification) {
  pChar->setValue((uint8_t*)&value,4);
  if(notification){pChar->notify();}
};

void ValueState::storeValue() {
  Settings.putUInt(_Name.c_str(),value);
};

void ValueState::setValue(uint32_t Value) {
  value=Value;
  storeValue();
  setBLEState();
};

void StringState::setBLEState(bool notification) {
  pChar->setValue((uint8_t*)str,str_size);
  if(notification){pChar->notify();}
};

void StringState::storeString() {
  Settings.putString(_Name.c_str(),str);//str could be changed to String
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

Sensor* Device::pSensor=NULL;
Device::States* Device::pStates=NULL;
Log* Device::pLog=NULL;

BoolState* Device::States::pMultiConnectState = NULL;
BoolState* Device::States::pSerialState = NULL;
BLEState* Device::States::pBLEState = NULL;
BoolState* Device::States::pLogState = NULL;
BoolState* Device::States::pEcoState = NULL;

ValueState* Device::States::pReadDelay = NULL;
StringState* Device::States::pLogFilePath = NULL;


Device::States::States() {
  /*Start the persistant memory management and get the module persistant state values*/
  //EEPROM.begin(EEPROM_SIZE);
  Settings.begin("settings");//Define the settings namespace

  /*Instantiate the settings according to the one contained in the EEPROM memory*/
  
  /* Create a setting to enable multiconnect for BLE 4.1 devices : Caractéristique pour activer les connections multiples pour les client BLE 4.1 minimum*/
  pMultiConnectState = new BoolState(MultiConnectStateUUID,"Multiconnect",1,presBool); 
  /* Create a setting to enable Serial : Caractéristique pour activer l'envoie des données par le port série*/
  pSerialState = new BoolState(SerialStateUUID,"Serial",1,presBool);
  /* Setting to enable or disable BLE Server (not possible through BLE)*/
  pBLEState = new BLEState(BLEStateUUID,"BLE",1,presBool);
  /*Create a setting to enable log to a file on onboard SD Card : Caractéristique pour activer l'enregistrement des mesures sur la carte SD*/
  pLogState = new BoolState(LogStateUUID,"Log",0,presBool);
  /*Create a setting to enable the eco mode that turn off the device and sensor between readings*/
  pEcoState = new BoolState(EcoStateUUID,"Eco",0,presBool);
  /*Setting to configure the to hold the timespan between two measurement: Creation d'une caractéristique contenant l'intervalle entre deux mesures : 4 octets*/
  pReadDelay = new ValueState(DelayUUID,"Delay",1,presDelay);
  /*Setting to  configure log  file path : Caractéristique pour configurer le nom du fichier de log*/
  pLogFilePath = new StringState(LogFilePathUUID,"Log File Path",40,presLogFilePath);
};

void Device::States::configBLEService(BLEService* pService) { //TO DO : Can be changed to a loop : for pState in {pMulticonnect, ...} {pState->initBLEState(pService);pState->pCallback;}
  /*Configure the BLE server to expose and configure the module settings : configuration du serveur pour exposer et configurer les paramètres du module en BLE*/
  pMultiConnectState->initBLEState(pService);
  pMultiConnectState->setBLEState(false);
  pMultiConnectState->pChar->setCallbacks(&StateCallback);
  
  pSerialState->initBLEState(pService);
  pSerialState->setBLEState(false);
  pSerialState->pChar->setCallbacks(&StateCallback);

  pLogState->initBLEState(pService);
  pLogState->setBLEState(false);
  pLogState->pChar->setCallbacks(&StateCallback);

  pEcoState->initBLEState(pService);
  pEcoState->setBLEState(false);
  pEcoState->pChar->setCallbacks(&StateCallback);

  pReadDelay->initBLEState(pService);
  pReadDelay->setBLEState(false);
  pReadDelay->pChar->setCallbacks(&StateCallback);

  pLogFilePath->initBLEState(pService);
  pLogFilePath->setBLEState(false);
  pLogFilePath->pChar->setCallbacks(&StateCallback);
  
  
};


Device::Device() {
  /*Instantiate the States nested class to recover the settings from EEPROM memory 
   Récupération des paramètres dans la mémoire EEPROM*/
  pStates = new States();
};

void Device::getSensor() {
  /* Get the plugged sensor through the value of the signature resistor
   * Découverte du capteur connecté grace à la valeur de résistance signature */
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
  /* Start the sensor power and initialize the sensor*/
  pSensor->powerOn();
  pSensor->init();
};

bool Device::isTimerWakeUp() {
  /* Get the wake up reason, to detect timer wake up of user wake up (switch off and on)
   * Détermination du mode de réveil : minuteur programmé ou bouton marche arrêt
   */
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER : 
      //Serial.println("Wakeup from sleep"); 
      return true;
    default : 
      //Serial.println("Wakeup from power on"); 
      return false;
     
    }
}

void Device::setBLEServer () {
  /*Init the BLE Server : Demarrage du serveur BLE fournissant les valeurs mesurées et les paramétres d'état du module*/
  /*Create the BLE Device : Creation du peripherique BLE et definition de son nom qui s'affichera lors du scan : peut contenir une reference unique egalement*/
  BLEDevice::init((deviceName + pSensor->getName() + "-" + deviceNumber).c_str());

  // Create the BLE Server : Creation du serveur BLE et mise en place de la fonction de callback pour savoir si le serveur est connecté et doit commencer à envoyer des notifications
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(&ServerCallback);

  // Create the BLE Services for the Environnemental Sensing Data and the Custom one: Creation du service pour les données environnementales et du service personnalisé
  pEnvService = pServer->createService(EnvServiceUUID,30); // The parameter indicate the max number of handles : 2 for a characteristic and 1 for a descriptor
  pCustomService = pServer->createService(CustomServiceUUID,30);
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

void Device::restartAdvertisingBLE() {
  if (pStates->pMultiConnectState->isOn()) { pAdvertising->start();}
};

void Device::startLog() {
  logTime=0;
  pLog = new Log(pSensor,pStates->pLogFilePath->str);
  pLog->initSD();
};

void Device::initSettings() {
  /* Init the services according to the settings of the device
   * Démarrage des services conformément aux paramètres du module
   */
  if (pStates->pLogState->isOn()) {
      pLog = new Log(pSensor,pStates->pLogFilePath->str);
      pLog->initSD();
      }
  if (pStates->pEcoState->isOn()) { /*Ecostate is there to prevent the init of the BLE Server when in EcoMode*/
      if(isTimerWakeUp()) {doReadAndSleep();} 
      } 
  if (pStates->pBLEState->isOn()) {
      esp_base_mac_addr_set(mac_adress);     
      setBLEServer();
      }       
  if (pStates->pSerialState->isOn()) {
      pSensor->printSerialHeader();
      }
  if (pStates->pEcoState->isOn()) {
      pStates->pEcoState->switchState();//WARNING : produce a crash with all settings , if the server initiate before this occured : TO INVESTIGATE AGAIN
      }
};

/*
 * Callbacks fonction launched when a value of the custom service is modified by a BLE client
 * Fonction définie par la bibliothèque est lancée lorsqu'une valeur a été modifiée par un client BLE
 */
void Device::States::StateCallbacks::onWrite (BLECharacteristic *pCharacteristic) { //TODO : remove this test by adding a callback for each State !!
  if (pCharacteristic == pMultiConnectState->pChar) {
      pMultiConnectState->switchState();
      if(pMultiConnectState->isOn()){pAdvertising->start();}
      }
  if (pCharacteristic == pSerialState->pChar) {pSerialState->switchState();}
  if (pCharacteristic == pLogState->pChar) {
      pLogState->switchState();
      if (pLogState->isOn()){startLog();}
      }
  if (pCharacteristic == pEcoState->pChar) {
      pEcoState->switchState();
      }
  uint8_t* pData = pCharacteristic->getData();
  if (pCharacteristic == pReadDelay->pChar) {
      memcpy(&pReadDelay->value,pData,4);
      pReadDelay->storeValue();
      }
  if (pCharacteristic == pLogFilePath->pChar) {
      memcpy(&pLogFilePath->str,pData,40);
      pLogFilePath->storeString();
      }
};

/*
   Callbacks fontion launched by the server when a connection or a disconnection occur
   Fonction definie par la bibliotheque qui est lancée lorsque l'état du serveur BLE change : événement : connexion et deconnexion
*/

void Device::ServerCallbacks::onConnect(BLEServer* pServer) {
      BLEConnected = true;
      restartAdvertisingBLE();//needed to support multi-connect(not done if multiconnect is not setted), that mean more than one client connected to server
      }; 

void Device::ServerCallbacks::onDisconnect(BLEServer* pServer) {
      BLEConnected = false;
      };

/*Function to deal with an incoming Serial change and set the device according to it : fonction pour gérer les commandes envoyée à travers l'USB*/

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
      if(pStates->pMultiConnectState->isOn()){pAdvertising->start();}
      break;
    case 'S' :
      pStates->pSerialState->switchState();
      break;
    case 'B' :
      pStates->pBLEState->switchState();  
      if (pStates->pBLEState->isOn()) {stopBLEServer();}
      else {setBLEServer();};
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
        pStates->pLogFilePath->str_size=incomingString.length();
        incomingString.toCharArray(pStates->pLogFilePath->str,pStates->pLogFilePath->str_size+1);
        pStates->pLogFilePath->storeString();
        }
      pStates->pLogState->switchState();
      if (pStates->pLogState->isOn()){startLog();}
      break;
    case 'R' :
      pLog->readFile();
      Serial.println("****************END*******************");
      break; 
  }
};

bool Device::doRead() {
  logTime+=pStates->pReadDelay->value; //add the delay to total log time for the next read
  if (pSensor->readData()){ //true if new datas are collected by sensor : vrai si des nouvelles données envoyées par le capteur sont disponibles    if (Serial.available()) {checkSerial();}
    if (pStates->pSerialState->isOn()){pSensor->printSerialData(&logTime);} // if Serial is on : print data to serial USB
    if (BLEConnected) {pSensor->setBLEData();} // if a BLE device is connected advertise the new value
    if (pStates->pLogState->isOn()) {pLog->logSD(&logTime);} // if Log on log to the current log file
    if (pStates->pEcoState->isOn()) { //going to sleep when Eco state has been turned on
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
  //TimerDelay-=micros();
  if (TimerDelay > micros()) {TimerDelay-=micros();} //If the sleep is launched remove the execution time to the delay : DONT WORK AT FIRST LAUNCH WITH SERIAL ? TOO LONG SERIAL?
  esp_sleep_enable_timer_wakeup(TimerDelay);
  Serial.println("Going to sleep now for : ");
  Serial.println(TimerDelay);
  Serial.flush(); 
  esp_deep_sleep_start();
};
