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

MHZ16Sensor::MHZ16Sensor():
  CO2(
    BLEUUID("0000486c-1000-2000-3000-6578706c6f72"), // Custom UUID for the CO2 rate
    "Carbon Dioxyd Rate",
    presentationCO2,
    (uint8_t*)&dCO2,
    4
    ),
  Temp(
    BLEUUID((uint16_t)0x2A6E), //0x2A6E is the characteristic for Temperature from ENV : en degres celsius correspond a un : sint16, Decimal,-2
    "Temperature",
    presentationTemp,
    (uint8_t*)&dTemp,
    2   
    )
    {}
    
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
  CO2.initCharacteristic(pEnvService);
  Temp.initCharacteristic(pEnvService);
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
  CO2.setCharacteristic();
  Temp.setCharacteristic();
}
