
#include "Explord_BLE.h"
/*
 * Class to store all the BLE definition of characteristics and notifying of new values
 *
 */
/*
 class BLESensor {
  
 };
*/

bool BLEChar::initCharacteristic(BLEService* pService) {
  _pService = pService;
  _pChar = _pService->createCharacteristic(_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
  Serial.println("Char created");
  _pChar->addDescriptor(new BLE2902());
  if (_CharName != "") {
    BLEDescriptor *nameDescriptor = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description : pour indiquer le nom de la valeur mesurée
    _pChar->addDescriptor(nameDescriptor);
    nameDescriptor->setValue(_CharName.c_str());
    Serial.println("Char Name created");
  }
  if (_presentation != NULL) {
    BLEDescriptor *presentationDescriptor = new BLEDescriptor((uint16_t)0x2904);
    _pChar->addDescriptor(presentationDescriptor);
    presentationDescriptor->setValue(_presentation, sizeof &_presentation);
    Serial.println("Char Presentation created");
  }
 };

bool BLEChar::setCharacteristic() {
  Serial.println("Value set for a char");
  _pChar->setValue((uint8_t*)_value, sizeof(&_value)); 
  _pChar->notify();;
};
