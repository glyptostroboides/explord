/* DHT Data
 *  The DHT library provide two values from the sensor temperature and humidity with one decimal.
 *  The library provide also computed heat index and dew point which are also exposed through BLE
 *  Datas sended are : humidity, temperature, dew point and heat index.
 */

#include "DHT22Sensor.h"
  
void DHT22Sensor::initSensor() {
   /* Init the I2C connection to the DHT sensor
   */
    dht.setup(DHTDataPin, DHTesp::DHT22); // pin for the data DHT I2C connection, then type of sensor DHT11, DHT22 etc...
    };

bool DHT22Sensor::getData() {
  TempAndHumidity DHTData = dht.getTempAndHumidity(); //get the temperature and humidity
  if (dht.getStatus() != 0) {
    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
    return false; // mostly due to a disconnected sensor
  }
  
  uint16_t _humidity = (uint16_t) (DHTData.humidity*100);
  //uint8_t _h[]= {(uint8_t)_humidity,(uint8_t)(_humidity >> 8)}; 
  Humidity.setValue((uint8_t*)&_humidity,2);
  int16_t _temperature = (int16_t) (DHTData.temperature*100);
  Temp.setValue((uint8_t*)&_temperature,2);
  int16_t _dew = (int16_t) (dht.computeDewPoint(DHTData.temperature, DHTData.humidity)*100);
  Dew.setValue((uint8_t*)&_dew,2);
  int16_t _heat = (int16_t) (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity)*100);
  Heat.setValue((uint8_t*)&_heat,2);
  Humidity.setSValue(String(DHTData.humidity));
  Temp.setSValue(String(DHTData.temperature));
  Dew.setSValue(String(dht.computeDewPoint(DHTData.temperature, DHTData.humidity)));
  Heat.setSValue(String (dht.computeHeatIndex(DHTData.temperature, DHTData.humidity)));
  return true;
}
