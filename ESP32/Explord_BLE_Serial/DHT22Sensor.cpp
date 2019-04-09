/* DHT Data
 *  The DHT library provide two values from the sensor temperature and humidity with one decimal.
 *  The library provide also computed heat index and dew point which are also exposed through BLE
 *  Datas sended are : humidity, temperature, dew point and heat index.
 */

#include "DHT22Sensor.h"

 /*Store the presentation format for each characteristics of DHT*/
uint8_t DHT22Sensor::presentationHumidity[] = {
    0x06, // Format = 6 = "unsigned 16-bit integer"
    0x02, // Exponent = 2
    0xAD, // Unit = 0x27AD = "percentage" (low byte)
    0x27, // ditto (high byte)
    0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
    0x00, // Description = 0 = "unknown" (low byte)
    0x00, // ditto (high byte) 
  };
uint8_t DHT22Sensor::presentationTemp[] = {
    0x0E, // Format = E = "signed 16-bit integer"
    0x02, // Exponent = 2
    0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
    0x27, // ditto (high byte)
    0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
    0x00, // Description = 0 = "unknown" (low byte)
    0x00, // ditto (high byte) 
  };
uint8_t DHT22Sensor::presentationDew[] = {
    0x0E, // Format = E = "signed 16-bit integer"
    0x02, // Exponent = 2
    0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
    0x27, // ditto (high byte)
    0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
    0x00, // Description = 0 = "unknown" (low byte)
    0x00, // ditto (high byte) 
  };
uint8_t DHT22Sensor::presentationHeat[] = {
  0x0E, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
  };

DHT22Sensor::DHT22Sensor(): 
  Humidity(
    BLEUUID((uint16_t)0x2A6F), // 0x2A6F : relative humidity in % correspond a un : uint16 ,Decimal, -1, soit 2 char 
    "Humidity",
    presentationHumidity,
    (uint8_t*)&dHumidity,
    2
    ),
  Temp(
    BLEUUID((uint16_t)0x2A6E), //0x2A6E is the characteristic for Temperature from ENV : en degres celsius correspond a un : sint16, Decimal,-2
    "Temperature",
    presentationTemp,
    (uint8_t*)&dTemp,
    2   
    ),
  Dew(
    BLEUUID((uint16_t)0x2A7B), // Dew Point in Celsius degrees with two decimals int
    "Dew Point",
    presentationDew,
    (uint8_t*)&dDew,
    2
    ),
  Heat(
    BLEUUID((uint16_t)0x2A7A), // Heat Index in Celsius degrees
    "Heat Index",
    presentationHeat,
    (uint8_t*)&dHeat,
    2
    )
    {}

void DHT22Sensor::powerOn() {
    /*
      * Power On the DHT when in use
      */
    pinMode(DHTPowerPin,OUTPUT);
    digitalWrite(DHTPowerPin,HIGH); // power on the DHT
    };
    
void DHT22Sensor::initSensor() {
   /* Init the I2C connection to the DHT sensor
   */
    dht.setup(DHTDataPin, DHTesp::DHT22); // pin for the data DHT I2C connection, then type of sensor DHT11, DHT22 etc...
    };

bool DHT22Sensor::getData() {
  DHTData = dht.getTempAndHumidity(); //get the temperature and humidity
  if (dht.getStatus() != 0) {
    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
    return false; // mostly due to a disconnected sensor
  }
  dTemp= (int16_t) (DHTData.temperature*100);
  dHumidity= (uint16_t) (DHTData.humidity*100);
  dHeat= (int16_t) (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity)*100);
  dDew= (int16_t) (dht.computeDewPoint(DHTData.temperature, DHTData.humidity)*100);
  sTemp=String(DHTData.temperature);
  sHumidity=String(DHTData.humidity); 
  sHeat= String (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity));
  sDew= String(dht.computeDewPoint(DHTData.temperature, DHTData.humidity));
  return true;
}

void DHT22Sensor::configEnvService(BLEService* pEnvService) {
  // Create BLE Characteristics : Creation des caractéristiques dans le service des données environnementales
  Humidity.initCharacteristic(pEnvService);
  Temp.initCharacteristic(pEnvService);
  Dew.initCharacteristic(pEnvService);
  Heat.initCharacteristic(pEnvService);
 
}

void DHT22Sensor::printSerialHeader() {
  Serial.println("Humidity,Temperature,Dew Point,Heat Index");

}

void DHT22Sensor::printSerialData() {
  Serial.println(String(sHumidity+","+sTemp+","+sDew+","+sHeat));
}

void DHT22Sensor::setBLEData() {
  //Define new value and notify to connected client : Definition et notification des nouvelles valeurs 
  Humidity.setCharacteristic();
  Temp.setCharacteristic();
  Dew.setCharacteristic();
  Heat.setCharacteristic();
}
