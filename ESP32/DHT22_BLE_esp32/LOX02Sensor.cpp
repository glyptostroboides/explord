
#include "LOX02Sensor.h"

 
/*
 *Format presentation descriptor
 *Définition des format de données pour le descripteur 0x2904 conforme à la norme BLE GATT
 */

uint8_t LOX02Sensor::presentationPPO2[] = {
  0x08, // Format = 8 = "unsigned 32-bit integer"
  0x01, // Exponent = 1
  0x24, // Unit = 0x2724 = "pressure (pascal)" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t LOX02Sensor::presentationTemp[] = {
  0x0E, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t LOX02Sensor::presentationPressure[] = {
  0x08, // Format = 8 = "unsigned 32-bit integer"
  0x01, // Exponent = 1
  0x24, // Unit = 0x2724 = "pressure (pascal)" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t LOX02Sensor::presentationO2[] = {
  0x06, // Format = 6 = "unsigned 16-bit integer"
  0x02, // Exponent = 2
  0xAD, // Unit = 0x27AD = "percentage" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};



void LOX02Sensor::powerOn() {
    /*
      * Power On the LOX02 when in use
      */
    pinMode(LOX02PowerPin,OUTPUT);
    digitalWrite(LOX02PowerPin,HIGH); // power on the LOX02
    };
    
void LOX02Sensor::initSensor() {
    /* Init the UART connection to the Luminox sensor
    *  Baud Rate, Format,RX,TX)
    */
    Serial1.begin(9600,SERIAL_8N1,LOX02RXPin,LOX02TXPin); // RX then TX (connect luminox pin 22 of ESP32 to RX of LOX, and pin 21 of ESP32 to TX of LOX)
    
    /*
     * Mise en reserve d'espace pour stocker la chaine de caracteres emise par le capteur LOX-02
     * Les caracteres '\n' et '\r' compte pour un char
     *   O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
     */
     
    LoxD.reserve(41); // just keep the bytes for the stored data : reservation de la memoire pour la chaine de caracteres : correspond au nombre de caracteres de la chaine envoyee par le LOX  
    };




bool LOX02Sensor::getData() {
  if(Serial1.available()){ //true if new datas are sended by LOX sensor : vrai si des nouvelles données envoyées par le LOX sont disponibles
      LoxD = Serial1.readStringUntil('\r');  // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n : format des données récupérées
      return true;
  }
}

void LOX02Sensor::configEnvService(BLEService* pEnvService) {
 
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
  namePPO2Descriptor->setValue(PPO2_CHARACTERISTIC_DESCRIPTION.c_str());
  
  BLEDescriptor *nameO2Descriptor = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description : pour indiquer le nom de la valeur mesurée
  pO2->addDescriptor(nameO2Descriptor);
  nameO2Descriptor->setValue(O2_CHARACTERISTIC_DESCRIPTION.c_str());
  
  
 
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

}

void LOX02Sensor::printSerialHeader() {
  Serial.println("Dioxygen Rate, Temperature,Pressure,Dioxygen PPressure");

}

void LOX02Sensor::printSerialData() {
  //Data extraction for the Serial port print to csv string: Extraction des donnees de la chaine de caracteres envoyee par le LOX-O2 pour le port Serie en csv
  Serial.println(String(LoxD.substring(28,33)+","+LoxD.substring(12,17)+","+LoxD.substring(20,24)+","+LoxD.substring(3,9)));
}

void LOX02Sensor::setBLEData() {
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
