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
  0x06, // Format = E = "signed 16-bit integer"
  0x02, // Exponent = 2
  0x2F, // Unit = 0x272F = "Celsius temperature" (low byte)
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte) 
  };

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
  //pTemp = pEnvService->createCharacteristic(TEMP_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE );
  pHumidity = pEnvService->createCharacteristic(HUMIDITY_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  pTemp = pEnvService->createCharacteristic(TEMP_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  pDew = pEnvService->createCharacteristic(DEW_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY  );
  pHeat = pEnvService->createCharacteristic(HEAT_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  
  // Create a BLE Descriptor with BLE2902 (which manage the Notify settings)
  pHumidity->addDescriptor(new BLE2902());
  pTemp->addDescriptor(new BLE2902());
  pDew->addDescriptor(new BLE2902());
  pHeat->addDescriptor(new BLE2902());
  
 //Define the presentation format for each characteristic ( Characteristic Presentation Format) : Définition des descripteurs contenant les informations sur la presentation des valeurs mesurées
  BLEDescriptor *presentationHumidityDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pHumidity->addDescriptor(presentationHumidityDescriptor);
  presentationHumidityDescriptor->setValue(presentationHumidity, sizeof presentationHumidity);
 
  BLEDescriptor *presentationTempDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pTemp->addDescriptor(presentationTempDescriptor);
  presentationTempDescriptor->setValue(presentationTemp, sizeof presentationTemp);

  BLEDescriptor *presentationDewDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pDew->addDescriptor(presentationDewDescriptor);
  presentationDewDescriptor->setValue(presentationDew, sizeof presentationDew);

  BLEDescriptor *presentationHeatDescriptor = new BLEDescriptor((uint16_t)0x2904);
  pHeat->addDescriptor(presentationHeatDescriptor);
  presentationHeatDescriptor->setValue(presentationHeat, sizeof presentationHeat);
}

void DHT22Sensor::printSerialHeader() {
  Serial.println("Humidity, Temperature,Dew Point,Heat Index");

}

void DHT22Sensor::printSerialData() {
  Serial.println(String(sHumidity+","+sTemp+","+sDew+","+sHeat));
}

void DHT22Sensor::setBLEData() {
  //Define new value and notify to connected client : Definition et notification des nouvelles valeurs 
  pHumidity->setValue((uint8_t*)&dHumidity, sizeof(dHumidity)); 
  pHumidity->notify();
  pTemp->setValue((uint8_t*)&dTemp, sizeof(dTemp)); // changed to work with temperature characteristic was 4 before
  pTemp->notify();
  pDew->setValue((uint8_t*)&dDew, sizeof(dDew)); 
  pDew->notify();
  pHeat->setValue((uint8_t*)&dHeat,  sizeof(dHeat)); 
  pHeat->notify();
}
