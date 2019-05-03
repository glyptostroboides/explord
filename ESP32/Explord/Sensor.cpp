
#include "Sensor.h"
#include "Drivers.h"

/*
   Stockage des formats de présentation des différentes unités pour les caractéristiques
   Store the presentation format for each characteristics
*/

uint8_t presCelsius[] = {
  0x0E, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

uint8_t presPercent[] = {
  0x06, // Format = 6 = "unsigned 16-bit integer"
  0x02, // Exponent = 2
  0xAD, // Unit = 0x27AD = "percentage" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

uint8_t presPascal[] = {
  0x08, // Format = 8 = "unsigned 32-bit integer"
  0x01, // Exponent = 1
  0x24, // Unit = 0x2724 = "pressure (pascal)" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

uint8_t presPPM[] = {
  0x08, // Format = 8 = "unsigned 32-bit integer"
  0x00, // Exponent = 0
  0xC4, // Unit = 0x27C4 = "concentration (parts per million)" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

/*
   Definition des valeurs mesurées disponibles pour les différents capteurs
   Define the available characteristics for all sensors
*/

Characteristic Sensor::Temp(
  BLEUUID((uint16_t)0x2A6E), //0x2A6E is the characteristic for Temperature from ENV : en degres celsius correspond a un : sint16, Decimal,-2
  "Temperature",
  presCelsius,
  2);

Characteristic Sensor::Humidity( // defined by the BLE GATT
  BLEUUID((uint16_t)0x2A6F), // 0x2A6F : relative humidity in % correspond a un : uint16 ,Decimal, -1, soit 2 char
  "Humidity",
  presPercent,
  2);

Characteristic Sensor::Dew(
  BLEUUID((uint16_t)0x2A7B), // Dew Point in Celsius degrees with two decimals int
  "Dew Point",
  presCelsius,
  2);

Characteristic Sensor::Heat(
  BLEUUID((uint16_t)0x2A7A), // Heat Index in Celsius degrees
  "Heat Index",
  presCelsius,
  2);

Characteristic Sensor::PPO2(
  BLEUUID("0000486b-1000-2000-3000-6578706c6f72"), //O2 partial pressure in millibar with 1 decimal converted to Pascals with no decimals
  "Dioxygen Partial Pressure",
  presPascal,
  4);

Characteristic Sensor::O2(
  BLEUUID("0000486a-1000-2000-3000-6578706c6f72"),
  "Dioxygen Rate",
  presPercent,
  2);

Characteristic Sensor::Pressure(
  BLEUUID((uint16_t)0x2A6D), // 0x2A6D : Pressure in pascal correspond a un : uint32 ,Decimal, -1, soit 2 char
  "Pressure",
  presPascal,
  4);

Characteristic Sensor::CO2(
  BLEUUID("0000486c-1000-2000-3000-6578706c6f72"), // Custom UUID for the CO2 rate
  "Carbon Dioxyd Rate",
  presPPM,
  4);

void Sensor::powerOn() {
  /*
      Power On the sensor when in use
  */
  pinMode(PowerPin, OUTPUT);
  digitalWrite(PowerPin, HIGH); // power on the sensor
}

void Sensor::powerOff() {
  digitalWrite(PowerPin, LOW); //power off the sensor
}

void Sensor::configBLEService(BLEService* pService) {
  for (int n = 0; n < CharNb; n++) {
    CharSet[n]->initBLECharacteristic(pService);
  }
}

void Sensor::setBLEData() {
  for (int n = 0; n < CharNb; n++) {
    CharSet[n]->setBLECharacteristic();
  }
}

void Sensor::printSerialHeader() {
  /*Serial.print(CharSet[0]->getName());
  for (int n = 1; n < CharNb; n++) {
    Serial.print(String("," + CharSet[n]->getName()));
  }
  Serial.println("");*/
  Serial.println(printHeader());
}

void Sensor::printSerialData() {
  /*Serial.print(CharSet[0]->getSValue());
  for (int n = 1; n < CharNb; n++) {
    Serial.print(String("," + CharSet[n]->getSValue()));
  }
  Serial.println("");*/
  Serial.println(printStringData());
}

String Sensor::printStringData() {
  String Data = CharSet[0]->getSValue();
  for (int n = 1; n < CharNb; n++) {Data.concat(String("," + CharSet[n]->getSValue()));}
  return Data;
}

String Sensor::printHeader() {
  String Header = CharSet[0]->getName();
  for (int n = 1; n < CharNb; n++) {Header.concat(String("," + CharSet[n]->getName()));}
  return Header;
}

/*
   DHT22 Specific Code
*/

//
//void Sensor::initDHT() {
//  /* Init the I2C connection to the DHT sensor*/
//  dht.setup(DataPin, DHTesp::DHT22);
//  // pin for the data DHT I2C connection, then type of sensor DHT11, DHT22 etc...
//  Name = String("DHT");
//  CharNb = 4;
//  //Sensor::CharSet[]={&Humidity,&Temp,&Dew,&Heat};
//  CharSet[0] = &Humidity;
//  CharSet[1] = &Temp;
//  CharSet[2] = &Dew;
//  CharSet[3] = &Heat;
//}
//
//bool Sensor::readDHT() {
//  TempAndHumidity DHTData = dht.getTempAndHumidity(); //get the temperature and humidity
//  if (dht.getStatus() != 0) {
//    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
//    return false; // mostly due to a disconnected sensor
//  }
//  delay(1);
//  uint16_t _humidity = (uint16_t) (DHTData.humidity * 100);
//  //uint8_t _h[] = {(uint8_t)(_humidity >> 8), (uint8_t)_humidity};
//  CharSet[0]->setValue((uint8_t*)&_humidity, 2);
//  int16_t _temperature = (int16_t) (DHTData.temperature * 100);
//  CharSet[1]->setValue((uint8_t*)&_temperature, 2);
//  int16_t _dew = (int16_t) (dht.computeDewPoint(DHTData.temperature, DHTData.humidity) * 100);
//  CharSet[2]->setValue((uint8_t*)&_dew, 2);
//  int16_t _heat = (int16_t) (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity) * 100);
//  CharSet[3]->setValue((uint8_t*)&_heat, 2);
//  CharSet[0]->setSValue(String(DHTData.humidity));
//  CharSet[1]->setSValue(String(DHTData.temperature));
//  CharSet[2]->setSValue(String(dht.computeDewPoint(DHTData.temperature, DHTData.humidity)));
//  CharSet[3]->setSValue(String (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity)));
//  return true;
//}


void Sensor::initSerial() {
  /* Init the UART connection for LOX02 or MHZ16
  Baud Rate, Format,RX,TX)*/
  Serial1.begin(9600, SERIAL_8N1, RXPin, TXPin); // RX then TX (connect sensor pin 19 RX of ESP32 to TX of MHZ16(pin5), and pin 21 TX of ESP32 to RX of MHZ16(pin6))
}

/*
   LOX-O2 Specific Code

*/
//
//void Sensor::initLOX() {
//  initSerial();
//  Name = String("LOX");
//  CharNb = 4;
//  CharSet[0] = &O2;
//  CharSet[1] = &Temp;
//  CharSet[2] = &Pressure;
//  CharSet[3] = &PPO2;
//  Serial.println("LOX initialized");
//}
//
//bool Sensor::readLOX() {
//  if (Serial1.available()) { //true if new datas are sended by LOX sensor : vrai si des nouvelles données envoyées par le LOX sont disponibles
//    delay(1);
//    String LoxD = Serial1.readStringUntil('\r');  // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n : format des données récupérées
//    // Data extraction for the BLE server : Extraction des donnees de la chaine de caracteres envoyee par le LOX-02 pour le BLE
//    // O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
//    uint16_t _O2 = String(LoxD.substring(28, 30) + LoxD.substring(31, 33)).toInt(); // extract from % xxx.xx :xxxx O2 rate in percent with two decimal
//    CharSet[0]->setValue((uint8_t*)&_O2, 2);
//    CharSet[0]->setSValue(LoxD.substring(28, 33));
//    //String(LoxD.substring(28,33)+","+LoxD.substring(12,17)+","+LoxD.substring(20,24)+","+LoxD.substring(3,9)));
//    int16_t _Temp = String(LoxD.substring(13, 15) + LoxD.charAt(16) + "0").toInt(); //extract from T yxx.x : xxx0 (add a zero to conform to BLE GATT)
//    if (LoxD.charAt(12) == '-') {
//      _Temp = -1 * _Temp; // deal with the sign if negative T value
//    }
//    CharSet[1]->setValue((uint8_t*)&_Temp, 2);
//    CharSet[1]->setSValue(LoxD.substring(12, 17));
//    uint32_t _Pressure = String(LoxD.substring(20, 24) + "000").toInt(); // extract from P xxxx : xxxx and convert from millibar to pascal (x100)with one decimal(x10)
//    CharSet[2]->setValue((uint8_t*)&_Pressure, 4);
//    CharSet[2]->setSValue(LoxD.substring(20, 24));
//    uint32_t _PPO2 = String(LoxD.substring(3, 7) + LoxD.charAt(8) + "00").toInt(); // extract from O xxxx.x : xxxxx in millibar with one decimal converted to pascal with one decimal
//    CharSet[3]->setValue((uint8_t*)&_PPO2, 4);
//    CharSet[3]->setSValue(LoxD.substring(3, 9));
//    return true;
//  }
//}
//
///*
//   MHZ-16 Specific Code
//*/
//
//void Sensor::initMHZ() {
//  initSerial();
//  Name = String("MHZ");
//  CharNb = 2;
//  CharSet[0] = &CO2;
//  CharSet[1] = &Temp;
//  Serial.println("MHZ initialized");
//}
//
//bool Sensor::readMHZ() {
//  /*
//    Command data for get the sensor values
//  */
//  const unsigned char GetSensorCommand[9] =
//  {
//    0xff, 0x01, 0x86, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x79
//  };
//  /*Store the bytes sended from MHZ16*/
//  uint8_t MHZData[9]; // Store the 9 bytes sended by the MHZ16
//  Serial1.write((uint8_t*)&GetSensorCommand, sizeof(GetSensorCommand)); // Send an array of nine bytes to MHZ16 to get the values in response
//  delay(10);
//  if (Serial1.available()) { //true if new datas are sended by MHZ16 sensor : vrai si des nouvelles données envoyées par le MHZ16 sont disponibles
//    delay(1);
//    Serial1.readBytes(MHZData, sizeof(MHZData)); // 9 bytes corresponding to the bytes sended by MHZ16 : format des données récupérées
//    if ((1 + (0xFF ^ (uint8_t)(MHZData[1] + MHZData[2] + MHZData[3] + MHZData[4] + MHZData[5] + MHZData[6] + MHZData[7]))) != MHZData[8]) //Byte 9 contain a checksum : all bytes summed, result inverted and add 1
//    {
//      Serial.println("MHZ Checksum failed");
//      return false;
//    }
//    uint32_t _CO2 = (int)MHZData[2] * 256 + (int)MHZData[3];
//    CharSet[0]->setValue((uint8_t*)&_CO2, 4);
//    CharSet[0]->setSValue(String(_CO2));
//    int16_t _Temp = ((int)MHZData[4] - 40) * 100; // value without decimals multiplied by 100 to add the two unsignificant decimals
//    CharSet[1]->setValue((uint8_t*)&_Temp, 2);
//    CharSet[1]->setSValue(String(float(_Temp / 100)));
//    return true;
//  }
//}
