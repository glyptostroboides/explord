/*
This program send environmental data through BLE with an ESP32.
The UUID used are the one from the BLE GATT specifications : https://www.bluetooth.com/specifications/gatt
*/ 

/*
 * This part is not specific to any sensor. BLE Server and Services that are used by all sensorts
 * 
 */

/*Define the pin for builtin LED : Definition de la broche pour la led intégrée 22 et non 21 comme l'indique le peu de documentation*/
#define LED 22

/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


/*
 *BLE Server pointer and Environnmental Sensing Service
 *Declaration du serveur BLE et du service environnemental
 */
BLEServer* pServer = NULL;
static BLEService *pEnvService = NULL;
/*
 *Value to store the BLE server connection state
 *Valeurs d'états de la connection BLE pour déterminer si il faut emettre les notifications ou non et recommencer a signale le capteur pour le BLE 4.1
 */
 
bool deviceConnected = false;
bool oldDeviceConnected = false;

/*
 * Callbacks fontion launched by the server when a connection or a disconnection occur
 * Fonction definie par la bibliotheque qui est lancée lorsque l'état du serveur BLE change : événement : connexion et deconnexion
 */
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      //BLEDevice::startAdvertising(); // needed to support multi-connect, that mean more than one client connected to server, must be commented if using BLE 4.0 device
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

/* DHT Data
 *  The DHT library provide two values from the sensor temperature and humidity with one decimal.
 *  The library provide also computed heat index and dew point which are also exposed through BLE
 *  Datas sended are : humidity, temperature, dew point and heat index.

 */

/*
 * Define the pins used for the datas and power of DHT22
 * Définition de la broche utilisée pour les données du DHT22
 */
 
#define DHTDataPin 17
#define DHTPowerPin 4

const uint16_t DHTDelay = 1000;
const String DHTDeviceName = "ExplordDHT";

#include <DHTesp.h>
  
  DHTesp dht;
  TempAndHumidity DHTData;
  uint16_t dHumidity;
  int16_t dTemp; 
  int16_t dHeat;
  int16_t dDew;

  String sTemp;
  String sHumidity;
  String sHeat;
  String sDew;

/*
 *Server pointer and BLE characteristics pointersfor the DHT
 *Declaration des pointeurs pour les caracteristiques BLE 
 */

static BLECharacteristic* pTemp = NULL;
static BLECharacteristic* pHumidity = NULL;
static BLECharacteristic* pHeat = NULL;
static BLECharacteristic* pDew = NULL;



// Definitions des valeurs pour le service "donnees environnementales" conforme aux definitions de la norme BLE

#define ENV_SERVICE_UUID  BLEUUID((uint16_t)0x181A) // 0x181A is the service for Environnemental Sensing : service pour les capteurs environnementaux
#define TEMP_UUID  BLEUUID((uint16_t)0x2A6E) // 0x2A6E is the characteristic for Temperature from ENV : en degres celsius correspond a un : sint16, Decimal,-2, soit 2 ouchar4 char
#define HUMIDITY_UUID BLEUUID((uint16_t)0x2A6F) // 0x2A6F : relative humidity in % correspond a un : uint16 ,Decimal, -1, soit 2 char 
#define DEW_UUID BLEUUID((uint16_t)0x2A7B) // Dew Point in Celsius degrees with two decimals int
#define HEAT_UUID BLEUUID((uint16_t)0x2A7A) // Heat Index in Celsius degrees




/*
 *Format presentation descriptor
 *Définition des format de données pour le descripteur 0x2904 conforme à la norme BLE GATT
 */

uint8_t presentationHumidity[] = {
  0x06, // Format = 6 = "unsigned 16-bit integer"
  0x02, // Exponent = 2
  0xAD, // Unit = 0x27AD = "percentage" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t presentationTemp[] = {
  0x0E, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t presentationDew[] = {
  0x0E, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t presentationHeat[] = {
  0x06, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};


void powerOnDHT() {
  /*
   * Power On the DHT when in use
   */
  pinMode(DHTPowerPin,OUTPUT);
  digitalWrite(DHTPowerPin,HIGH); // power on the DHT
}

void initDHT() {
   /* Init the I2C connection to the DHT sensor
   */
    dht.setup(DHTDataPin, DHTesp::DHT22); // pin for the data DHT I2C connection, then type of sensor DHT11, DHT22 etc...

}

bool getDHTData() {
  DHTData = dht.getTempAndHumidity(); //get the temperature and humidity
  if (dht.getStatus() != 0) {
    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
    return false; // mostly due to a disconnected sensor
  }
  dTemp= (int16_t) (DHTData.temperature*100);
  dHumidity= (uint16_t) (DHTData.humidity*100);
  dHeat= (int16_t) (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity)*100);
  dDew= (int16_t) (dht.computeDewPoint(DHTData.temperature, DHTData.humidity)*100);
  sTemp=String(DHTData.temperature);
  sHumidity=String(DHTData.humidity); 
  sHeat= String (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity));
  sDew= String(dht.computeDewPoint(DHTData.temperature, DHTData.humidity));
  return true;
}

void configDHTEnvService() {
 
  // Create BLE Characteristics : Creation des caractéristiques dans le service des données environnementales
  //pTemp = pEnvService->createCharacteristic(TEMP_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE );
  pHumidity = pEnvService->createCharacteristic(HUMIDITY_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  pTemp = pEnvService->createCharacteristic(TEMP_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  pDew = pEnvService->createCharacteristic(DEW_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY  );
  pHeat = pEnvService->createCharacteristic(HEAT_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  
  // Create a BLE Descriptor with BLE2902 (which manage the Notify settings)
  pHumidity->addDescriptor(new BLE2902());
  pTemp->addDescriptor(new BLE2902());
  pDew->addDescriptor(new BLE2902());
  pHeat->addDescriptor(new BLE2902());
  
 //Define the presentation format for each characteristic ( Characteristic Presentation Format) : Définition des descripteurs contenant les informations sur la presentation des valeurs mesurées
  BLEDescriptor *presentationHumidityDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pHumidity->addDescriptor(presentationHumidityDescriptor);
  presentationHumidityDescriptor->setValue(presentationHumidity, sizeof presentationHumidity);
 
  BLEDescriptor *presentationTempDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pTemp->addDescriptor(presentationTempDescriptor);
  presentationTempDescriptor->setValue(presentationTemp, sizeof presentationTemp);

  BLEDescriptor *presentationDewDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pDew->addDescriptor(presentationDewDescriptor);
  presentationDewDescriptor->setValue(presentationDew, sizeof presentationDew);

  BLEDescriptor *presentationHeatDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pHeat->addDescriptor(presentationHeatDescriptor);
  presentationHeatDescriptor->setValue(presentationHeat, sizeof presentationHeat);
}

void printDHTSerialHeader() {
  Serial.println("Humidity, Temperature,Dew Point,Heat Index");

}

void printDHTSerialData() {
  Serial.println(String(sHumidity+","+sTemp+","+sDew+","+sHeat));
}

void setDHTBLEData() {
  //Define new value and notify to connected client : Definition et notification des nouvelles valeurs 
  pHumidity->setValue((uint8_t*)&dHumidity, sizeof(dHumidity)); 
  pHumidity->notify();
  pTemp->setValue((uint8_t*)&dTemp, sizeof(dTemp)); // changed to work with temperature characteristic was 4 before
  pTemp->notify();
  pDew->setValue((uint8_t*)&dDew, sizeof(dDew)); 
  pDew->notify();
  pHeat->setValue((uint8_t*)&dHeat,  sizeof(dHeat)); 
  pHeat->notify();
}



void setup() {
  pinMode(LED,OUTPUT);
  /*
   * Define the power pin for the DHT in order to get it of when not connected
   * Définition de la broche pour alimenter le DHT quand il est utilisé mais pas lorsque le capteur est en charge
   * Cela permet également de téléverser avec le composant soudé sinon il faudrait le déconnecter
   */
  powerOnDHT();
  /* Init the serial connection through USB
   * Demarrage de la connection serie a travers le port USB
   */
  Serial.begin(115200);
  
  initDHT();

    //Init the BLE Server : Demarrage du serveur BLE
  // Create the BLE Device : Creation du peripherique BLE et definition de son nom qui s'affichera lors du scan : peut contenir une reference unique egalement
  BLEDevice::init(DHTDeviceName.c_str());

  // Create the BLE Server : Creation du serveur BLE et mise en place de la fonction de callback pour savoir si le serveur est connecté et doit commencer à envoyer des notifications
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service for the Environnemental Sensing Data : Creation du service pour les données environnementales
  pEnvService = pServer->createService(ENV_SERVICE_UUID);
  
  configDHTEnvService();
   
  // Start the service : Demarrage des services sur les données environnementales
  pEnvService->start();

  // Start advertising : Demarrage des notifications pour le client
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(ENV_SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  //Serial.println("Waiting a client connection to notify... : En attente d'une connection BLE pour notifier");
  printDHTSerialHeader();
  delay(DHTDelay); // The DHT need about 1 second to calculate new values : Il faut laisser du temps au DHT pour calculer l'humidité et la température
}

void loop() {
    if (getDHTData()){ //true if new datas are collected by DHT sensor : vrai si des nouvelles données envoyées par le DHT sont disponibles
                digitalWrite(LED,HIGH);
		            printDHTSerialData();

      if (deviceConnected) { // if a BLE device is connected : si un peripherique BLE est connecté
                //Define new value and notify to connected client : Definition et notification des nouvelles valeurs 
                setDHTBLEData();  
            }
            delay(100);
            digitalWrite(LED,LOW);
            delay(DHTDelay-100); // The DHT22 need about 1 second to calculate new values : pour le DHT22 il faut au moins 1 seconde

    }
        
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready : si le client n'est pas connecté le capteur retente de proposer des données
        pServer->startAdvertising(); // restart advertising
        Serial.println("Restart advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
	      Serial.println("Connection to a BLE client done");	
        oldDeviceConnected = deviceConnected;
    }
   
}
