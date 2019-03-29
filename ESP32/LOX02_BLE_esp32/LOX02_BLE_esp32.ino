/*
This program send luminox LOX-02 data through BLE with an ESP32.
Datas sended are : O2 partial pressure, temperature, pressure and O2 rate
The UUID used are the one from the BLE GATT specifications : https://www.bluetooth.com/specifications/gatt
*/ 

/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* Hardware Serial is used to define the Serial ports on the ESP32 that has three serial port
// Inclusion de la bibliotheque HardWareSerial qui permet la gestion d autres ports series avec le microcontrolleur ESP32*/
#include <HardwareSerial.h> // add the library to connect with O2 UART sensor through serial


/* LUMINOX DATA
 * The Luminox sensor is by default in stream mode so it send a new string every second
 * Le capteur Luminox LOX-02 est par defaut en mode flux continue, il emet donc une nouvelle chaine de caracteres toute les secondes environ
 * The format of this data are : 
 * Le format de ces donnees est le suivant : 
  O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
  O : dioxygen partial pressure : ppO2 in mbar , pression partielle en dioxygene en millibar
  T : temperature in °C : where y is - or + : temperature en degres Celsius, y est - ou +
  P : pressure in mbar : pression en millibar
  % : dioxygen rate in percent : pourcentage de dioxygene dans l'air
  e : sensor state : 0000 if good, else contact SST Sensing ! : etat du capteur 0000 si il n y pas de souci et sinon il faut contacter le fabricant
   */
   
//HardwareSerial Serial1(1); // RX (need interrupt capacity), TX, Serial1 is already defined in HardwareSerial.h
String LoxD = ""; // init the value for the LOX data string

//Define the value to collect from the LoxD
//unsigned int dPPO2; // O2 partial pressure in millibar with 1 decimal converted to Pascals with no decimals
uint32_t dPPO2;

//signed short dTemp; // temperature in Celsius degrees with 2 decimals : signed value
int16_t dTemp; 

//unsigned int dPressure; // pressure in millibar with 0 decimal converted to Pascals with no decimals
uint32_t dPressure;

//unsigned int dO2; //%  O2 rate in percent with 2 decimals
uint16_t dO2;
// sO2 = "000.00" : possible peut être pour reserver directement l'espace necessaire pour stocker la donnee

/*
 *Server pointer and BLE characteristics pointers
 *Declaration du serveur et des pointeurs pour les caracteristiques BLE 
 */
BLEServer* pServer = NULL;
BLECharacteristic* pPPO2 = NULL;
BLECharacteristic* pTemp = NULL;
BLECharacteristic* pPressure = NULL;
BLECharacteristic* pO2 = NULL;

/*
 *Value to store the BLE server connection state
 *Valeurs d'états de la connection BLE pour déterminer si il faut emettre les notifications ou non
 */
bool deviceConnected = false;
bool oldDeviceConnected = false;

/*
 *Format presentation descriptor
 *Définition des format de données pour le descripteur 0x2904 conforme à la norme BLE GATT
 */

uint8_t presentationPPO2[] = {
  0x08, // Format = 8 = "unsigned 32-bit integer"
  0x01, // Exponent = 1
  0x24, // Unit = 0x2724 = "pressure (pascal)" (low byte)
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
uint8_t presentationPressure[] = {
  0x08, // Format = 8 = "unsigned 32-bit integer"
  0x01, // Exponent = 1
  0x24, // Unit = 0x2724 = "pressure (pascal)" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t presentationO2[] = {
  0x06, // Format = 6 = "unsigned 16-bit integer"
  0x02, // Exponent = 2
  0xAD, // Unit = 0x27AD = "percentage" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};



// Definitions des valeurs pour le service "donnees environnementales" conforme aux definitions de la norme BLE

#define ENV_SERVICE_UUID  BLEUUID((uint16_t)0x181A) // 0x181A is the service for Environnemental Sensing : service pour les capteurs environnementaux
#define TEMP_UUID  BLEUUID((uint16_t)0x2A6E) // 0x2A6E is the characteristic for Temperature from ENV : en degres celsius correspond a un : sint16, Decimal,-2, soit 2 ouchar4 char
#define PRESSURE_UUID BLEUUID((uint16_t)0x2A6D) // 0x2A6D : Pressure in pascal correspond a un : uint32 ,Decimal, -1, soit 2 char 

/*Autres caracteristiques potentiellement utiles dans le projet dans le sercice "données environnementales"
0x2A6F : Humidity : pourcentage avec une precision de 0.01%, 

//Autres caracteristiques definies hors du service donnees environnementales conforme aux definitions de la norme BLE
//#define TEMPERATURE_CELSIUS_UUID BLEUUID((uint16_t)0x2A1F) // 0x2A1F characteristic for Temperature Celsius
0x2A37 : Heart rate measurement : en battement par minute , uint8 
Ox2A19 : Battery Level : niveau de batterie en pourcentage , uint8

*/
/*
 * Universal Unique ID definitions
 * Definitions des identifiants uniques des caractéristiques des donnees sur le dioxygene (generer au hasard et hors norme rendant chaque service et caracteristique unique)
 * Il serait possible de generer un identifiant unique par valeur mesurer par un capteur
*/
#define PPO2_UUID "0000486b-1000-2000-3000-6578706c6f72"
#define PPO2_CHARACTERISTIC_DESCRIPTION "Dioxygen Partial Pressure"
#define O2_UUID "0000486a-1000-2000-3000-6578706c6f72"
#define O2_CHARACTERISTIC_DESCRIPTION "Dioxygen Rate"

/*
 * Callbacks fontion launched by the server when a connection or a disconnection occur
 * Fonction definie par la bibliotheque qui est lancée lorsque l'état du serveur BLE change : événement : connexion et deconnexion
 */
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising(); // needed to support multi-connect, that mean more than one client connected to server
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  /* Init the serial connection through USB
   * Demarrage de la connection serie a travers le port USB
   */
  Serial.begin(115200);
  
  /* Init the UART connection to the Luminox sensor
   *  Baud Rate, Format,RX,TX)
   */
  Serial1.begin(9600,SERIAL_8N1,19,21); // RX then TX (connect luminox pin 19 of ESP32 to RX of LOX, and pin 21 of ESP32 to TX of LOX)

  /*
   * Mise en reserve d'espace pour stocker la chaine de caracteres emise par le capteur LOX-02
   * Les caracteres '\n' et '\r' compte pour un char
   *   O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
   */
   
  LoxD.reserve(41); // just keep the bytes for the stored data : reservation de la memoire pour la chaine de caracteres : correspond au nombre de caracteres de la chaine envoyee par le LOX

  //Init the BLE Server : Demarrage du serveur BLE
  // Create the BLE Device : Creation du peripherique BLE et definition de son nom qui s'affichera lors du scan : peut contenir une reference unique egalement
  BLEDevice::init("ExplordO2");

  // Create the BLE Server : Creation du serveur BLE et mise en place de la fonction de callback pour savoir si le serveur est connecté et doit commencer à envoyer des notifications
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service for the Environnemental Sensing Data : Creation du service pour les données environnementales
  BLEService *pEnvService = pServer->createService(ENV_SERVICE_UUID);

  // Create BLE Characteristics : Creation des caractéristiques dans le service des données environnementales
  //pTemp = pEnvService->createCharacteristic(TEMP_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE );
  pPPO2 = pEnvService->createCharacteristic(PPO2_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  pTemp = pEnvService->createCharacteristic(TEMP_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  pPressure = pEnvService->createCharacteristic(PRESSURE_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY  );
  pO2 = pEnvService->createCharacteristic(O2_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  
 
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor with BLE2902 (which manage the Notify settings)
  pPPO2->addDescriptor(new BLE2902());
  pTemp->addDescriptor(new BLE2902());
  pPressure->addDescriptor(new BLE2902());
  pO2->addDescriptor(new BLE2902());
  

 
  // Define a Descriptor for the name of the value for PPO2 and O2 : Definition des descripteurs contenant le nom des valeurs présentées par le serveur
  BLEDescriptor *namePPO2Descriptor = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description : pour indiquer le nom de la valeur mesurée
  pPPO2->addDescriptor(namePPO2Descriptor);
  namePPO2Descriptor->setValue(PPO2_CHARACTERISTIC_DESCRIPTION);
  
  BLEDescriptor *nameO2Descriptor = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description : pour indiquer le nom de la valeur mesurée
  pO2->addDescriptor(nameO2Descriptor);
  nameO2Descriptor->setValue(O2_CHARACTERISTIC_DESCRIPTION);
  
  
 
 //Define the presentation format for each characteristic ( Characteristic Presentation Format) : Définition des descripteurs contenant les informations sur la presentation des valeurs mesurées
  BLEDescriptor *presentationPPO2Descriptor = new BLEDescriptor((uint16_t)0x2904);
  pPPO2->addDescriptor(presentationPPO2Descriptor);
  presentationPPO2Descriptor->setValue(presentationPPO2, sizeof presentationPPO2);
 
  BLEDescriptor *presentationTempDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pTemp->addDescriptor(presentationTempDescriptor);
  presentationTempDescriptor->setValue(presentationTemp, sizeof presentationTemp);

  BLEDescriptor *presentationPressureDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pPressure->addDescriptor(presentationPressureDescriptor);
  presentationPressureDescriptor->setValue(presentationPressure, sizeof presentationPressure);

  BLEDescriptor *presentationO2Descriptor = new BLEDescriptor((uint16_t)0x2904);
  pO2->addDescriptor(presentationO2Descriptor);
  presentationO2Descriptor->setValue(presentationO2, sizeof presentationO2);

  
  // Start the service : Demarrage des services sur les données environnementales
  pEnvService->start();

  // Start advertising : Demarrage des notifications pour le client
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(ENV_SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  //Serial.println("Waiting a client connection to notify... : En attente d'une connection BLE pour notifier");
  Serial.println("Dioxygen Rate, Temperature,Pressure,Dioxygen PPressure");
}

void loop() {
    // notify changed value
    if (Serial1.available()){ //true if new datas are sended by LOX sensor : vrai si des nouvelles données envoyées par le LOX sont disponibles
      LoxD = Serial1.readStringUntil('\r');  // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n : format des données récupérées
      //Data extraction for the Serial port print to csv string: Extraction des donnees de la chaine de caracteres envoyee par le LOX-O2 pour le port Serie en csv
      //if (Serial.available()){

              //Serial.println("Dioxygen Rate, Temperature,Pressure,Dioxygen PPressure");
            Serial.println(String(LoxD.substring(28,33)+","+LoxD.substring(12,17)+","+LoxD.substring(20,24)+","+LoxD.substring(3,9)));
           // }
      if (deviceConnected) { // if a BLE device is connected : si un peripherique BLE est connecté
                // Data extraction for the BLE server : Extraction des donnees de la chaine de caracteres envoyee par le LOX-02 pour le BLE
                // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
               
                dPPO2 = String(LoxD.substring(3,7)+LoxD.charAt(8)+"00").toInt(); // extract from O xxxx.x : xxxxx in millibar with one decimal converted to pascal with one decimal
            
                dTemp = String(LoxD.substring(13,15)+LoxD.charAt(16)+"0").toInt(); //extract from T yxx.x : xxx0 (add a zero to conform to BLE GATT)
                if (LoxD.charAt(12) == '-'){dTemp = -1 * dTemp;} // deal with the sign if negative T value
            
                dPressure = String(LoxD.substring(20,24)+"000").toInt(); // extract from P xxxx : xxxx and convert from millibar to pascal (x100)with one decimal(x10)
            
                dO2 = String(LoxD.substring(28,30)+LoxD.substring(31,33)).toInt(); // extract from % xxx.xx :xxxx O2 rate in percent with two decimal
                
                //Define new value and notify to connected client : Definition et notification des nouvelles valeurs 
                pPPO2->setValue((uint8_t*)&dPPO2, sizeof(dPPO2)); 
                pPPO2->notify();
                pTemp->setValue((uint8_t*)&dTemp, sizeof(dTemp)); // changed to work with temperature characteristic was 4 before
                pTemp->notify();
                pPressure->setValue((uint8_t*)&dPressure, sizeof(dPressure)); 
                pPressure->notify();
                pO2->setValue((uint8_t*)&dO2,  sizeof(dO2)); 
                pO2->notify();
                
            }
            delay(990); // utilité à vérifier temp à calibrer et gérer

    }
        
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready : si le client n'est pas connecté le capteur retente de proposer des données
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
   
}
