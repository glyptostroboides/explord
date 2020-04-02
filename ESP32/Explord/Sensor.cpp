
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

uint8_t presLux[] = {
  0x06, // Format = 6 = "unsigned 16-bit integer"
  0x02, // Exponent = 2
  0x31, // Unit = 0x2731 = "illuminance (lux)" (low byte)
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

Characteristic Sensor::Illuminance(
  BLEUUID((uint16_t)0x2AFB), // 0x2AFB : Illuminance in lux : uint24 ?, -2
  "Illuminance",
  presLux,
  2);

void Sensor::powerOn() {
  /*
      Power On the sensor when in use
  */
  pinMode(PowerPin, OUTPUT);
  digitalWrite(PowerPin, HIGH); // power on the sensor
  delay(30);
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
  Serial.println(printHeader());
}

void Sensor::printSerialData(unsigned long * logTime) {
  Serial.println(printStringData(logTime));
}

String Sensor::printStringData(unsigned long * logTime) {
  String l_time = String(*logTime);
  String Data = String (l_time + ","+ CharSet[0]->getSValue());
  for (int n = 1; n < CharNb; n++) {Data.concat(String("," + CharSet[n]->getSValue()));}
  return Data;
}

String Sensor::printHeader() {
  String Header = CharSet[0]->getName();
  for (int n = 1; n < CharNb; n++) {Header.concat(String("," + CharSet[n]->getName()));}
  return Header;
}

void Sensor::initSerial() {
  /* Init the UART connection for LOX02 or MHZ16
  Baud Rate, Format,RX,TX)*/
  Serial1.begin(9600, SERIAL_8N1, RXPin, TXPin); // RX then TX (connect sensor pin 19 RX of ESP32 to TX of MHZ16(pin5), and pin 21 TX of ESP32 to RX of MHZ16(pin6))
}

void Sensor::initWire() {
  //Wire.begin(RXPin,TXPin); // Not Working : need to modify the library call to Wire
  TwoWire I2C = TwoWire(0);
  I2C.begin(RXPin, TXPin, 100000);
}
