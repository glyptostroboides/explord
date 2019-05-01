
#include "LOX02Sensor.h"
 
void LOX02Sensor::initLOXSensor() {
    /* Init the UART connection to the Luminox sensor
    *  Baud Rate, Format,RX,TX)
    */
    Serial1.begin(9600,SERIAL_8N1,Sensor::RXPin,Sensor::RTXPin); // RX then TX (connect luminox pin 22 of ESP32 to RX of LOX, and pin 21 of ESP32 to TX of LOX)
    
    /*
     * Mise en reserve d'espace pour stocker la chaine de caracteres emise par le capteur LOX-02
     * Les caracteres '\n' et '\r' compte pour un char
     *   O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
     */
     
    LoxD.reserve(41); // just keep the bytes for the stored data : reservation de la memoire pour la chaine de caracteres : correspond au nombre de caracteres de la chaine envoyee par le LOX  
    };

bool LOX02Sensor::getLOXData() {
  if(Serial1.available()){ //true if new datas are sended by LOX sensor : vrai si des nouvelles données envoyées par le LOX sont disponibles
    delay(1);
    LoxD = Serial1.readStringUntil('\r');  // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n : format des données récupérées
    // Data extraction for the BLE server : Extraction des donnees de la chaine de caracteres envoyee par le LOX-02 pour le BLE
    // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
    dPPO2 = String(LoxD.substring(3,7)+LoxD.charAt(8)+"00").toInt(); // extract from O xxxx.x : xxxxx in millibar with one decimal converted to pascal with one decimal
    dTemp = String(LoxD.substring(13,15)+LoxD.charAt(16)+"0").toInt(); //extract from T yxx.x : xxx0 (add a zero to conform to BLE GATT)
    if (LoxD.charAt(12) == '-'){dTemp = -1 * dTemp;} // deal with the sign if negative T value 
    dPressure = String(LoxD.substring(20,24)+"000").toInt(); // extract from P xxxx : xxxx and convert from millibar to pascal (x100)with one decimal(x10)
    dO2 = String(LoxD.substring(28,30)+LoxD.substring(31,33)).toInt(); // extract from % xxx.xx :xxxx O2 rate in percent with two decimal 
    return true;
  }
}
