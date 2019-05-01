#pragma once
/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* Hardware Serial is used to define the Serial ports on the ESP32 that has three serial port
// Inclusion de la bibliotheque HardWareSerial qui permet la gestion d autres ports series avec le microcontrolleur ESP32*/
#include <HardwareSerial.h> // add the library to connect with O2 UART sensor through serial
#include "Characteristic.h"
  
class MHZ16Sensor {
  private :
    /*
     * Command data for get the sensor values
     */
    const unsigned char GetSensorCommand[9]=
    {
    0xff, 0x01, 0x86, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x79
    };
   
    /*Store the bytes sended from MHZ16*/
    /* MHZ16 DATA
    */
    uint8_t MHZData[9]; // Store the 9 bytes sended by the MHZ16
  public :
    void initSensor();
    bool getData();
};
