
#include "BLE_Char.h"
/*
 * Class to store all the BLE definition of characteristics and notifying of new values
 *
 */

/*
 * Define the characteristic name, presentation to initialize the advertising
 * Définition du nom, du format des caractéristiques BLE
 */
bool BLEChar::initCharacteristic(BLEService* pService) {
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

bool BLEChar::setCharacteristic() {
  //Serial.println("Value set for a char");
  _pChar->setValue(_value,_value_size); 
  _pChar->notify();;
};
