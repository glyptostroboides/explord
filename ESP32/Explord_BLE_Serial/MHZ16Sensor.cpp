#include "MHZ16Sensor.h"
 
/*
 *Format presentation descriptor
 *Définition des format de données pour le descripteur 0x2904 conforme à la norme BLE GATT
 */

uint8_t MHZ16Sensor::presentationCO2[] = {
  0x08, // Format = 8 = "unsigned 32-bit integer"
  0x00, // Exponent = 0
  0xC4, // Unit = 0x27C4 = "concentration (parts per million)" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};
uint8_t MHZ16Sensor::presentationTemp[] = {
  0x0E, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
};

void MHZ16Sensor::powerOn() {
    /*
      * Power On the MHZ16 when in use
      */
    pinMode(PowerPin,OUTPUT);
    digitalWrite(PowerPin,HIGH); // power on the MHZ16
    };
    
void MHZ16Sensor::initSensor() {
    /* Init the UART connection to the MHZ16 sensor
    *  Baud Rate, Format,RX,TX)
    */
    Serial1.begin(9600,SERIAL_8N1,RXPin,TXPin); // RX then TX (connect sensor pin 19 RX of ESP32 to TX of MHZ16(pin5), and pin 21 TX of ESP32 to RX of MHZ16(pin6))
    };

bool MHZ16Sensor::getData() {
  Serial1.write((uint8_t*)&GetSensorCommand,sizeof(GetSensorCommand)); // Send an array of nine bytes to MHZ16 to get the values in response
  delay(10);
  if(Serial1.available()){ //true if new datas are sended by MHZ16 sensor : vrai si des nouvelles données envoyées par le MHZ16 sont disponibles
    delay(1);
    Serial1.readBytes(MHZData,sizeof(MHZData));  // 9 bytes corresponding to the bytes sended by MHZ16 : format des données récupérées
    if((1 + (0xFF ^ (uint8_t)(MHZData[1] + MHZData[2] + MHZData[3] + MHZData[4] + MHZData[5] + MHZData[6] + MHZData[7]))) != MHZData[8]) //Byte 9 contain a checksum : all bytes summed, result inverted and add 1
      {
        Serial.println("MHZ Checksum failed");
        return false;
      }
    dCO2 = (int)MHZData[2] * 256 + (int)MHZData[3];
    dTemp = ((int)MHZData[4] - 40)*100; // value without decimals multiplied by 100 to add the two unsignificant decimals 
    return true;
  }
}

void MHZ16Sensor::configEnvService(BLEService* pEnvService) {
 
  // Create BLE Characteristics : Creation des caractéristiques dans le service des données environnementales
  pCO2 = pEnvService->createCharacteristic(CO2_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pTemp = pEnvService->createCharacteristic(TEMP_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor with BLE2902 (which manage the Notify settings)
  pCO2->addDescriptor(new BLE2902());
  pTemp->addDescriptor(new BLE2902());
  
  // Define a Descriptor for the name of the value for CO2 : Definition des descripteurs contenant le nom des valeurs présentées par le serveur
  BLEDescriptor *nameCO2Descriptor = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description : pour indiquer le nom de la valeur mesurée
  pCO2->addDescriptor(nameCO2Descriptor);
  nameCO2Descriptor->setValue(CO2_CHARACTERISTIC_DESCRIPTION.c_str());
  
 //Define the presentation format for each characteristic ( Characteristic Presentation Format) : Définition des descripteurs contenant les informations sur la presentation des valeurs mesurées
  BLEDescriptor *presentationCO2Descriptor = new BLEDescriptor((uint16_t)0x2904);
  pCO2->addDescriptor(presentationCO2Descriptor);
  presentationCO2Descriptor->setValue(presentationCO2, sizeof presentationCO2);
 
  BLEDescriptor *presentationTempDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pTemp->addDescriptor(presentationTempDescriptor);
  presentationTempDescriptor->setValue(presentationTemp, sizeof presentationTemp);
}

void MHZ16Sensor::printSerialHeader() {
  Serial.println("Carbon Dioxyd Rate,Temperature");
}

void MHZ16Sensor::printSerialData() {
  //Data extraction for the Serial port print to csv string: Extraction des donnees de la chaine de caracteres envoyee par le LOX-O2 pour le port Serie en csv
  Serial.println(String(String(dCO2)+","+String(float(dTemp/100))));
}

void MHZ16Sensor::setBLEData() {
    
  //Define new value and notify to connected client : Definition et notification des nouvelles valeurs 
  pCO2->setValue((uint8_t*)&dCO2, sizeof(dCO2)); 
  pCO2->notify();
  pTemp->setValue((uint8_t*)&dTemp, sizeof(dTemp));
  pTemp->notify();
}
