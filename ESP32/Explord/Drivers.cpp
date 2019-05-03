#include "Drivers.h"
#include "Sensor.h"

void DHT::init(){
   /* Init the I2C connection to the DHT sensor*/
  dht.setup(DataPin, DHTesp::DHT22);
  // pin for the data DHT I2C connection, then type of sensor DHT11, DHT22 etc...
}

bool DHT::readData(){
  DHTData = dht.getTempAndHumidity(); //get the temperature and humidity
  if (dht.getStatus() != 0) {
    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
    return false; // mostly due to a disconnected sensor
  }
  delay(1);
  uint16_t _humidity = (uint16_t) (DHTData.humidity * 100);
  //uint8_t _h[] = {(uint8_t)(_humidity >> 8), (uint8_t)_humidity};
  CharSet[0]->setValue((uint8_t*)&_humidity, 2);
  int16_t _temperature = (int16_t) (DHTData.temperature * 100);
  CharSet[1]->setValue((uint8_t*)&_temperature, 2);
  int16_t _dew = (int16_t) (dht.computeDewPoint(DHTData.temperature, DHTData.humidity) * 100);
  CharSet[2]->setValue((uint8_t*)&_dew, 2);
  int16_t _heat = (int16_t) (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity) * 100);
  CharSet[3]->setValue((uint8_t*)&_heat, 2);
  CharSet[0]->setSValue(String(DHTData.humidity));
  CharSet[1]->setSValue(String(DHTData.temperature));
  CharSet[2]->setSValue(String(dht.computeDewPoint(DHTData.temperature, DHTData.humidity)));
  CharSet[3]->setSValue(String (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity)));
  return true;
}

void LOX::init(){
   /* Init the Serial connection to the LOX sensor*/
    initSerial();  
}

bool LOX::readData(){
  if (Serial1.available()) { //true if new datas are sended by LOX sensor : vrai si des nouvelles données envoyées par le LOX sont disponibles
    delay(1);
    String LoxD = Serial1.readStringUntil('\r');  // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n : format des données récupérées
    // Data extraction for the BLE server : Extraction des donnees de la chaine de caracteres envoyee par le LOX-02 pour le BLE
    // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
    uint16_t _O2 = String(LoxD.substring(28, 30) + LoxD.substring(31, 33)).toInt(); // extract from % xxx.xx :xxxx O2 rate in percent with two decimal
    CharSet[0]->setValue((uint8_t*)&_O2, 2);
    CharSet[0]->setSValue(LoxD.substring(28, 33));
    //String(LoxD.substring(28,33)+","+LoxD.substring(12,17)+","+LoxD.substring(20,24)+","+LoxD.substring(3,9)));
    int16_t _Temp = String(LoxD.substring(13, 15) + LoxD.charAt(16) + "0").toInt(); //extract from T yxx.x : xxx0 (add a zero to conform to BLE GATT)
    if (LoxD.charAt(12) == '-') {
      _Temp = -1 * _Temp; // deal with the sign if negative T value
    }
    CharSet[1]->setValue((uint8_t*)&_Temp, 2);
    CharSet[1]->setSValue(LoxD.substring(12, 17));
    uint32_t _Pressure = String(LoxD.substring(20, 24) + "000").toInt(); // extract from P xxxx : xxxx and convert from millibar to pascal (x100)with one decimal(x10)
    CharSet[2]->setValue((uint8_t*)&_Pressure, 4);
    CharSet[2]->setSValue(LoxD.substring(20, 24));
    uint32_t _PPO2 = String(LoxD.substring(3, 7) + LoxD.charAt(8) + "00").toInt(); // extract from O xxxx.x : xxxxx in millibar with one decimal converted to pascal with one decimal
    CharSet[3]->setValue((uint8_t*)&_PPO2, 4);
    CharSet[3]->setSValue(LoxD.substring(3, 9));
    return true;
  }
}

void MHZ::init(){
   /* Init the Serial connection to the MHZ sensor*/
    initSerial();  
}

bool MHZ::readData(){
  /*
    Command data for get the sensor values
  */
  const unsigned char GetSensorCommand[9] =
  {
    0xff, 0x01, 0x86, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x79
  };
  /*Store the bytes sended from MHZ16*/
  uint8_t MHZData[9]; // Store the 9 bytes sended by the MHZ16
  Serial1.write((uint8_t*)&GetSensorCommand, sizeof(GetSensorCommand)); // Send an array of nine bytes to MHZ16 to get the values in response
  delay(10);
  if (Serial1.available()) { //true if new datas are sended by MHZ16 sensor : vrai si des nouvelles données envoyées par le MHZ16 sont disponibles
    delay(1);
    Serial1.readBytes(MHZData, sizeof(MHZData)); // 9 bytes corresponding to the bytes sended by MHZ16 : format des données récupérées
    if ((1 + (0xFF ^ (uint8_t)(MHZData[1] + MHZData[2] + MHZData[3] + MHZData[4] + MHZData[5] + MHZData[6] + MHZData[7]))) != MHZData[8]) //Byte 9 contain a checksum : all bytes summed, result inverted and add 1
    {
      Serial.println("MHZ Checksum failed");
      return false;
    }
    uint32_t _CO2 = (int)MHZData[2] * 256 + (int)MHZData[3];
    CharSet[0]->setValue((uint8_t*)&_CO2, 4);
    CharSet[0]->setSValue(String(_CO2));
    int16_t _Temp = ((int)MHZData[4] - 40) * 100; // value without decimals multiplied by 100 to add the two unsignificant decimals
    CharSet[1]->setValue((uint8_t*)&_Temp, 2);
    CharSet[1]->setSValue(String(float(_Temp / 100)));
    return true;
  }
}
