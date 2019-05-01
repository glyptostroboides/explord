#include "MHZ16Sensor.h"
 
void MHZ16Sensor::initSensor() {
    /* Init the UART connection to the MHZ16 sensor
    *  Baud Rate, Format,RX,TX)
    */
    Serial1.begin(9600,SERIAL_8N1,Sensor::RXPin,Sensor::TXPin); // RX then TX (connect sensor pin 19 RX of ESP32 to TX of MHZ16(pin5), and pin 21 TX of ESP32 to RX of MHZ16(pin6))
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
