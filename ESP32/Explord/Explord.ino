/*
  This program send environmental data through BLE or Serial with an ESP32.
  The UUID used are the one from the BLE GATT specifications : https://www.bluetooth.com/specifications/gatt
*/
#include "Device.h" //Store all settings and function of the device
//#include "Configuration.h" //Store all configuration details
//#include "Drivers.h" //Manage the sensor
//#include "Log.h" //Manage the storage of values inside SD



/*Define the initial values of state of the device*/
unsigned long BlinkTime=BLINK_TIME;//time of the led blink in ms
bool LedOn=false; //led starts off
//RTC_DATA_ATTR char CurrentLogFile[20]=DEFAULT_LOG_FILE;//log file name, saved for deep sleep
//RTC_DATA_ATTR uint32_t readDelay = READ_DELAY; //Define a value for the delay between each reading : Définition de l'intervalle entre deux mesures en secondes

/*Define the timer to store the time for the main loop*/
unsigned long LedTime = 0; //timer for the blinking led
unsigned long DelayTime = 0; // timer for the sensor readings inside the main loop
unsigned long ReadvertisingTime = 0; //timer for readvertising in BLE
/*RTC_DATA_ATTR unsigned long logTime = 0; //Store the time between the begin of log to save or display it : temps depuis le début de l'acquisition à afficher ou à transmettre
*/
/*
  Value to store the BLE server connection state
  Valeurs d'états de la connection BLE pour déterminer si il faut emettre les notifications ou non et recommencer a signale le capteur pour le BLE 4.1
*/

//bool BLEConnected = false; //
//bool oldBLEConnected = false; //
 
/*
 * Define the UUID for the BLE GATT environnmental sensing service used by all sensors
 * Definition de l'identifiant unique pour le service des capteurs environnementaux
 */

//const BLEUUID EnvServiceUUID = BLEUUID((uint16_t)0x181A); // 0x181A is the service for Environnemental Sensing : service pour les capteurs environnementaux

/*
 * Define the UUID for the custom BLE Service called Explord service used by all sensors
 * Definition de l'identifiant unique pour le service BLE personnel utilisé par tous les capteurs et des identifiants pour ses valeurs
 */
//const BLEUUID CustomServiceUUID = BLEUUID("00004860-1000-2000-3000-6578706c6f72"); //like all custom characteristics start with 0000486*
//const BLEUUID DelayUUID = BLEUUID("00004861-1000-2000-3000-6578706c6f72");

//const BLEUUID MultiConnectStateUUID = BLEUUID("00004870-1000-2000-3000-6578706c6f72");
//const BLEUUID SerialStateUUID = BLEUUID("00004871-1000-2000-3000-6578706c6f72");
//const BLEUUID LogStateUUID = BLEUUID("00004872-1000-2000-3000-6578706c6f72");
//const BLEUUID BLEStateUUID = BLEUUID("00004873-1000-2000-3000-6578706c6f72");
//const BLEUUID EcoStateUUID = BLEUUID("00004874-1000-2000-3000-6578706c6f72");
/*
  BLE Server, Environnmental Sensing Service and Custon Service pointers and for the Sensor singleton
  Declaration des pointeurs pour le serveur BLE, le service des données environnementales, le service BLE personnel et le singleton de la classe Sensor
*/
//static BLEServer* pServer = NULL;//
//static BLEAdvertising* pAdvertising = NULL;//
//static BLEService *pEnvService = NULL, *pCustomService = NULL;//

/*Define the instance pointer of the sensor, the log file, ...*/
//Sensor* pSensor;//
//Log* pLog;//

/*
/*Define a value for the delay between each reading : Définition de l'intervalle entre deux mesures en secondes*/
//static BLECharacteristic* pDelay = NULL;

/*Store the delay and the current log file path into permanent memory*/
/*
void storeDelay() { 
  EEPROM.writeUInt(10,readDelay);
  EEPROM.commit();
  }
void storeLogFilePath() {
  EEPROM.writeString(20,CurrentLogFile);
  EEPROM.commit();
}

/*
 * Callbacks fonction launched when a value of the custom service is modified by a BLE client
 * Fonction définie par la bibliothèque est lancée lorsqu'une valeur a été modifiée par un client BLE
 */
/*
class ClientCallbacks: public BLECharacteristicCallbacks {//à gerer
    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* pData = pCharacteristic->getData();
      if(pCharacteristic == pDelay) {
          memcpy(&readDelay,pData,4);
          storeDelay();
          }
    }
};*/
/*
static ClientCallbacks* pClientCallbacks = NULL;

static BoolState* pMultiConnectState;//
static BoolState* pSerialState;//
static BLEState* pBLEState;//
static BoolState* pLogState;//
static BoolState* pEcoState;//

class StateCallbacks: public BLECharacteristicCallbacks { //
  void onWrite(BLECharacteristic *pCharacteristic) {
      if (pCharacteristic == pMultiConnectState->pChar) {
        pMultiConnectState->switchState();
        pAdvertising->start();
        }
      if (pCharacteristic == pSerialState->pChar) {pSerialState->switchState();}
      if (pCharacteristic == pLogState->pChar) {
        pLogState->switchState();
        if (pLogState->isOn()){ startLog(); }  
        }
      if (pCharacteristic == pEcoState->pChar) {pEcoState->switchState();}
    } 
};

static StateCallbacks* pStateCallbacks = NULL; //

/*
   Callbacks fontion launched by the server when a connection or a disconnection occur
   Fonction definie par la bibliotheque qui est lancée lorsque l'état du serveur BLE change : événement : connexion et deconnexion
*/
/*
class ServerCallbacks: public BLEServerCallbacks { //
    void onConnect(BLEServer* pServer) {
      BLEConnected = true;
      if (pMultiConnectState->isOn()) { BLEDevice::startAdvertising();} // needed to support multi-connect, that mean more than one client connected to server, must be commented if using BLE 4.0 device
    };

    void onDisconnect(BLEServer* pServer) {
      BLEConnected = false;
    }
};

uint8_t getPluggedSensor() { //
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
*/
void switchLed() {
  LedTime=millis();
  if(LedOn) {
    digitalWrite(LEDPIN, LOW);
    LedOn=false;
  }
  else {
    digitalWrite(LEDPIN, HIGH);
    LedOn=true;
  }
};
/*
void startLog() {//
  logTime=0;
  pLog = new Log(pSensor,CurrentLogFile);
  pLog->initSD();
};

void checkSerial() {
  String incomingString = Serial.readStringUntil('\r');
  char incomingOrder = incomingString.charAt(0);
  String incomingParameter = incomingString.substring(1);
  switch (incomingOrder) {
    case 'D' :
      readDelay=incomingParameter.toInt();
      storeDelay();
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
      if (pBLEState->isOn()) {
        stopBLEServer();
      }
      else {setBLEServer();};
      pBLEState->switchState();
      break;
    case 'E' :
      pEcoState->switchState();
      break;
    case 'L' :
      if (incomingString.charAt(1)==' ') {
        if (incomingString.charAt(2)=='/'){
          incomingString= incomingString.substring(2) + ".csv";
          }
        else {
          incomingString= "/" + incomingString.substring(2) + ".csv";
          }
        incomingString.toCharArray(CurrentLogFile,20);
        storeLogFilePath();
        }
      pLogState->switchState();
      if (pLogState->isOn()){
         startLog();
        }
      break;
    case 'R' :
      pLog->readFile();
      Serial.println("****************END*******************");
      break; 
  }
};

void setBLEServer() {//manque Delay et CurrentLogFile (sans BLE pour l'instant)//
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
  pDelay = pCustomService->createCharacteristic(DelayUUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pDelay->setCallbacks(pClientCallbacks);
  pDelay->setValue((uint8_t*)&readDelay,4);

  pStateCallbacks = new StateCallbacks();
  // Create a BLE characteristic that enable multiconnect for BLE 4.1 devices : Caractéristique pour activer les connections multiples pour les client BLE 4.1 minimum
  pMultiConnectState->initBLEState(pCustomService);
  pMultiConnectState->pChar->setCallbacks(pStateCallbacks);
  
  // Create a BLE characteristic that enable Serial : Caractéristique pour activer l'envoie des données par le port série
  pSerialState->initBLEState(pCustomService);
  pSerialState->pChar->setCallbacks(pStateCallbacks);

  // Create a BLE characteristic that enable log to a file on onboard SD Card : Caractéristique pour activer l'enregistrement des mesures sur la carte SD
  pLogState->initBLEState(pCustomService);
  pLogState->pChar->setCallbacks(pStateCallbacks);

  // Create a BLE characteristic to enable the eco mode that turn off the device and sensor between readings
  pEcoState->initBLEState(pCustomService);
  pEcoState->pChar->setCallbacks(pStateCallbacks);

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

void stopBLEServer() {//
  pAdvertising->stop();
};

void getDelay() { //To recover the delay from EEPROM
  readDelay=EEPROM.readUInt(10);
  if (!readDelay){readDelay=1;storeDelay();} //Only for the first start of the device or if delay has been set to 0 ?
};

void getLogFilePath() {
  EEPROM.readString(20,CurrentLogFile,20);
};
*/
/*
void getStates() {
   /*Start the EEPROM memory management and get the module persistant state values*/
  //EEPROM.begin(EEPROM_SIZE);

  /*Init the state of the device according to EEPROM values*/
  /*Recover the delay and current log file for now*/
  //getDelay();
  //getLogFilePath();
  /*Instantiate the boolean configuration stored in EEPROM*/
  
  //pStateCallbacks = new StateCallbacks();
  
  /*EEPROM.write(2,1); //turn BLE ON : There is a BUG with BLE off all switch states results in CRASH
  EEPROM.commit();*/
  /*
  pMultiConnectState = new BoolState(0,MultiConnectStateUUID,"Multiconnect");
  pSerialState = new BoolState(1,SerialStateUUID,"Serial");
  pBLEState = new BLEState(2,BLEStateUUID,"BLE");
  pLogState = new BoolState(3,LogStateUUID,"Log");
  pEcoState = new BoolState(4,EcoStateUUID,"Eco");
};

bool isTimerWakeUp() { //
  /* Get the wake up reason, to detect timer wake up of user wake up (switch off and on)
   * Détermination du mode de réveil : minuteur programmé ou bouton marche arrêt
   *//*
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER : 
      Serial.println("Wakeup from sleep"); 
      return true;
    default : 
      Serial.println("Wakeup from power on"); 
      return false;
    }
};

void initStates() {//
  /* Init the services according to the states of the device
   * Démarrage des services conformément aux valeurs d'états du module
   *//*
  if (pLogState->isOn()) {
      pLog = new Log(pSensor,CurrentLogFile);
      pLog->initSD();
      }
  if (pEcoState->isOn()) {
      if(!isTimerWakeUp()) {
        pEcoState->switchState();
        }
      else {doReadAndSleep();}
      }
  if (pBLEState->isOn()) {
      esp_base_mac_addr_set(mac_adress);     
      setBLEServer();
      }  
      
  if (pSerialState->isOn()) {pSensor->printSerialHeader();}
};

bool doRead() {//
  if (pSensor->readData()){ //true if new datas are collected by sensor : vrai si des nouvelles données envoyées par le capteur sont disponibles    if (Serial.available()) {checkSerial();}
    if (pSerialState->isOn()){pSensor->printSerialData(&logTime);} // if Serial is on : print data to serial USB
    if (BLEConnected) {pSensor->setBLEData();} // if a BLE device is connected
    if (pLogState->isOn()) {pLog->logSD(&logTime);} // if Log on log to the current log file
    if (pEcoState->isOn()) {
      logTime+=readDelay;
      doSleep();
      }
    return true;
  }
  else {return false;}
};

void doReadAndSleep() {//
  //delay(1000);
  if (doRead()) { doSleep();}
};

void doSleep() {//
  pSensor->powerOff();
  unsigned long TimerDelay = (readDelay *1000000);
  if (TimerDelay > micros()) {TimerDelay-=micros();}
  esp_sleep_enable_timer_wakeup(TimerDelay);
  Serial.println("Going to sleep now for : ");
  Serial.println(TimerDelay);
  Serial.flush(); 
  esp_deep_sleep_start();
};
*/

Device * pDevice;

void setup() {
  /*Set the internal led as an output for blinking purpose*/
  pinMode(LEDPIN, OUTPUT);
  
  /* Init the serial connection through USB
     Demarrage de la connection serie a travers le port USB
  */
  Serial.begin(115200);

  pDevice = new Device();
  /*Discover which sensor is connected,init the sensor and start it : détection, initialisation et démarrage du capteur connecté*/
  pDevice->getSensor();
  pDevice->startSensor();
  /* Init the services according to the settings of the device
    * Démarrage des services conformément aux paramètres du module
    */
  pDevice->initSettings();

  //delay(1000); // The sensor need about 1 second to calculate new values : Il faut laisser du temps au capteur pour calculer sa première valeur
  if(pDevice->doRead()) {switchLed();}
  DelayTime=millis();
}

void loop() {
  /*
   * Read the sensor data according to the delay and send it through BLE and Serial
   */
  if(millis() > DelayTime + (pDevice->pStates->pReadDelay->value*1000)) { //launched when the time since last measurement is higher than the readDelay
    logTime+=pDevice->pStates->pReadDelay->value; //add the delay to total log time for the next read
    /* Get the data from sensor and act according to settings and turn led on after last reading*/
    if(pDevice->doRead()) {switchLed();} //true if new datas are collected by sensor : vrai si des nouvelles données envoyées par le capteur sont disponibles
    DelayTime=millis(); //WARNING : issue when the delay is changed
  }
  /*Turn off the led lighted on after last reading*/
  if(LedOn and (millis() > LedTime + BlinkTime)) { switchLed();}
  
  /*
   * BLE Connecting and Disconnecting stuff
   */
  // disconnecting // give the bluetooth stack the chance to get things ready : si aucun client n'est connecté le capteur retente de proposer des données après 500ms
  if (!pDevice->BLEConnected and pDevice->oldBLEConnected and (millis()> ReadvertisingTime+ 500 )) {
    ReadvertisingTime=millis();
    pDevice->startAdvertisingBLE(); // restart advertising when disconnected
    pDevice->oldBLEConnected = pDevice->BLEConnected;
  }
  // connecting
  if (pDevice->BLEConnected && !pDevice->oldBLEConnected) { pDevice->oldBLEConnected = pDevice->BLEConnected; }// Connection to a BLE client done : connection à un client BLE effectuée
  
  /*
   * Serial stuff to read the incoming settings and order through USB Serial port
   */
  if (Serial.available()) {pDevice->getSerial();}
  
}
