/*
This program send environmental data through BLE with an ESP32.
The UUID used are the one from the BLE GATT specifications : https://www.bluetooth.com/specifications/gatt
*/ 

/*
 * This part is not specific to any sensor. BLE Server and Services that are used by all sensorts
 * 
 */

/*Define the pin for builtin LEDPin : Definition de la broche pour la  intégrée 22 et non 21 comme l'indique le peu de documentation*/
const int LEDPin = 22;

/*Define a value for the delay between each mesure : Définition de l'intervalle entre deux mesures en millisecondes*/

uint16_t readingsDelay = 1000;
String deviceName = "Explord";
String deviceNumber = "01"; //added to the Sensor specific device name

/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/*Define the UUID for the environnmental sensing service used by all sensors*/

const BLEUUID EnvServiceUUID = BLEUUID((uint16_t)0x181A); // 0x181A is the service for Environnemental Sensing : service pour les capteurs environnementaux

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

#include "DHT22Sensor.h"

DHT22Sensor Sensor;

void setup() {
  
  pinMode(LEDPin,OUTPUT);
  /*
   * Define the power pin for the DHT in order to get it of when not connected
   * Définition de la broche pour alimenter le DHT quand il est utilisé mais pas lorsque le capteur est en charge
   * Cela permet également de téléverser avec le composant soudé sinon il faudrait le déconnecter
   */
  Sensor.powerOn();
  /* Init the serial connection through USB
   * Demarrage de la connection serie a travers le port USB
   */
  Serial.begin(115200);
  
  Sensor.initSensor();

    //Init the BLE Server : Demarrage du serveur BLE
  // Create the BLE Device : Creation du peripherique BLE et definition de son nom qui s'affichera lors du scan : peut contenir une reference unique egalement
  BLEDevice::init((deviceName+Sensor.getName()+"-"+deviceNumber).c_str());

  // Create the BLE Server : Creation du serveur BLE et mise en place de la fonction de callback pour savoir si le serveur est connecté et doit commencer à envoyer des notifications
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service for the Environnemental Sensing Data : Creation du service pour les données environnementales
  pEnvService = pServer->createService(EnvServiceUUID);
  
  Sensor.configEnvService(pEnvService);
   
  // Start the service : Demarrage des services sur les données environnementales
  pEnvService->start();

  // Start advertising : Demarrage des notifications pour le client
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(EnvServiceUUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  //Serial.println("Waiting a client connection to notify... : En attente d'une connection BLE pour notifier");
  Sensor.printSerialHeader();
  delay(readingsDelay); // The DHT need about 1 second to calculate new values : Il faut laisser du temps au DHT pour calculer l'humidité et la température
}

void loop() {
    if (Sensor.getData()){ //true if new datas are collected by DHT sensor : vrai si des nouvelles données envoyées par le DHT sont disponibles
                digitalWrite(LEDPin,HIGH);
		            Sensor.printSerialData();

      if (deviceConnected) { // if a BLE device is connected : si un peripherique BLE est connecté
                //Define new value and notify to connected client : Definition et notification des nouvelles valeurs 
                Sensor.setBLEData();  
            }
            delay(100);
            digitalWrite(LEDPin,LOW);
            delay(readingsDelay-100); // The DHT22 need about 1 second to calculate new values : pour le DHT22 il faut au moins 1 seconde

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
