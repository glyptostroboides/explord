
#include "BLE_Char.h"
/*
 * Class to store all the BLE definition of characteristics and notifying of new values
 *
 */


String BLEChar::getCharName(){
  return BLEChar::_CharName;
}

/*
uint8_t* BLEChar::getCharValue(){
  return BLEChar::_dvalue;
}
*/
void BLEChar::setCharValue(uint8_t* d_value, size_t value_size){
  for (int n=0;n<value_size;n++) {
    //BLEChar::_dvalue[n]= *(d_value + n);
    BLEChar::_value[n]=d_value[n];
    //Serial.println(BLEChar::_value[n],HEX);
  }
}

String BLEChar::getCharSValue(){
  return BLEChar::_svalue;
}

void BLEChar::setCharSValue(String s_value){
  BLEChar::_svalue= s_value;
}

/*
 * Define the characteristic name, presentation to initialize the advertising
 * Définition du nom, du format des caractéristiques BLE
 */
bool BLEChar::initBLECharacteristic(BLEService* pService) {
    // Create BLE Characteristics : Creation des caractéristiques dans le service des données environnementales
    //Define a pointer for the characteristic
  _pChar = pService->createCharacteristic(_UUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor with BLE2902 (which manage the Notify settings)
  _pChar->addDescriptor(new BLE2902());
  // Define a Descriptor for the name of the characteristic as a string: Definition des descripteurs contenant le nom de la valeur présentée par le serveur
  if (_CharName != "") {
    BLEDescriptor *nameDescriptor = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description : pour indiquer le nom de la valeur mesurée
    _pChar->addDescriptor(nameDescriptor);
    nameDescriptor->setValue(_CharName.c_str());
  }
  if (_presentation != NULL) {
  //Define the presentation format for each characteristic ( Characteristic Presentation Format) : Définition des descripteurs contenant les informations sur la presentation des valeurs mesurées  if (_presentation != NULL) {
    BLEDescriptor *presentationDescriptor = new BLEDescriptor((uint16_t)0x2904);
    _pChar->addDescriptor(presentationDescriptor);
    presentationDescriptor->setValue(_presentation, 7);
  }
 };

/*
 * Set the value of a BLE Characteristic according to the new value given by sensor and notity this change to client
 * Modifie la valeur de la caractéristique BLE et notifie le client de ce changement
 */

bool BLEChar::setBLECharacteristic() {
  //Serial.println("Value set for a char");
  _pChar->setValue(_value,_value_size); 
  _pChar->notify();
};

/*
bool BLEChar::setBLECharacteristicBis() {
  //Serial.println("Value set for a char");
  _pChar->setValue(_dvalue,_value_size); 
  _pChar->notify();
};
*/
