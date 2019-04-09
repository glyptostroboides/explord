/* BLE for ESP32 default library on ESP32-arduino framework
/ Inclusion des bibliotheques BLE pour l'environnement ESP-32 Arduino*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/* Hardware Serial is used to define the Serial ports on the ESP32 that has three serial port
// Inclusion de la bibliotheque HardWareSerial qui permet la gestion d autres ports series avec le microcontrolleur ESP32*/
#include <HardwareSerial.h> // add the library to connect with O2 UART sensor through serial
#include "BLE_Char.h"
  
class MHZ16Sensor {
  private :
    const String SensorName = "MHZ";
    /*
    * Define the pins used for the datas and power of MHZ16
    * Définition de la broche utilisée pour les données du MHZ16
    */
    const uint8_t RXPin = 19;
    const uint8_t TXPin = 21;
    const uint8_t PowerPin = 4;

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

    uint32_t dCO2; //CO2 rate in ppm
    int16_t dTemp; // temperature in Celsius degrees with 0 decimals : signed value

    /*
    *Format presentation descriptor
    *Définition des format de données pour le LOX02 pour le descripteur 0x2904 conforme à la norme BLE GATT
    */
    static uint8_t presentationTemp[7];
    static uint8_t presentationCO2[7];
   
    BLEChar CO2;
    BLEChar Temp ;  
  public :
    MHZ16Sensor();
    String getName(){return SensorName;};
    void powerOn();
    void initSensor();
    bool getData();
    void configEnvService(BLEService* pEnvService);
    void printSerialHeader();
    void printSerialData();    
    void setBLEData();   
};
