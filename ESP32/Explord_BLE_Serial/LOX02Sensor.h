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
  
class LOX02Sensor {
     
    /*Store the string sended from DHT*/
    /* LUMINOX DATA
    * The Luminox sensor is by default in stream mode so it send a new string every second
    * Le capteur Luminox LOX-02 est par defaut en mode flux continue, il emet donc une nouvelle chaine de caracteres toute les secondes environ
    * The format of this data are : 
    * Le format de ces donnees est le suivant : 
      O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
      O : dioxygen partial pressure : ppO2 in mbar , pression partielle en dioxygene en millibar
      T : temperature in °C : where y is - or + : temperature en degres Celsius, y est - ou +
      P : pressure in mbar : pression en millibar
      % : dioxygen rate in percent : pourcentage de dioxygene dans l'air
      e : sensor state : 0000 if good, else contact SST Sensing ! : etat du capteur 0000 si il n y pas de souci et sinon il faut contacter le fabricant
   */
    String LoxD = ""; // init the value for the LOX data string

    //Define the value to collect from the LoxD
 
    //uint32_t dPPO2; //O2 partial pressure in millibar with 1 decimal converted to Pascals with no decimals
    //int16_t dTemp; // temperature in Celsius degrees with 2 decimals : signed value
    //uint32_t dPressure; // pressure in millibar with 0 decimal converted to Pascals with no decimals
    //uint16_t dO2;//%  O2 rate in percent with 2 decimals
    
    /*
    *BLE characteristics pointers for the LOX02
    *Declaration des pointeurs pour les caracteristiques BLE 
    */
    
  public :
    void initSensor();
    bool getData();   
};
