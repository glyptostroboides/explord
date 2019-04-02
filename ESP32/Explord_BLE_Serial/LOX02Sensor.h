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
  private :
    const String SensorName = "LOX";
    /*
    * Define the pins used for the datas and power of LOX02
    * Définition de la broche utilisée pour les données du LOX02
    */
    const uint8_t RXPin = 19;
    const uint8_t TXPin = 21;
    const uint8_t PowerPin = 4;
   
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
 
    uint32_t dPPO2; //O2 partial pressure in millibar with 1 decimal converted to Pascals with no decimals
    int16_t dTemp; // temperature in Celsius degrees with 2 decimals : signed value
    uint32_t dPressure; // pressure in millibar with 0 decimal converted to Pascals with no decimals
    uint16_t dO2;//%  O2 rate in percent with 2 decimals
    
    /*
    *BLE characteristics pointers for the LOX02
    *Declaration des pointeurs pour les caracteristiques BLE 
    */
    BLECharacteristic* pPPO2 = NULL;
    BLECharacteristic* pTemp = NULL;
    BLECharacteristic* pPressure = NULL;
    BLECharacteristic* pO2 = NULL;
    
    /* 
    *  Define the UUID for the  characteritic used by the LOX02 Sensor
    *  Definitions des identifiants pour le LOX02 pour le service "donnees environnementales" conforme aux definitions de la norme BLE
    */
    const BLEUUID TEMP_UUID = BLEUUID((uint16_t)0x2A6E); // 0x2A6E is the characteristic for Temperature from ENV : en degres celsius correspond a un : sint16, Decimal,-2, soit 2 ouchar4 char
    const BLEUUID PRESSURE_UUID = BLEUUID((uint16_t)0x2A6D); // 0x2A6D : Pressure in pascal correspond a un : uint32 ,Decimal, -1, soit 2 char 
    const BLEUUID PPO2_UUID = BLEUUID("0000486b-1000-2000-3000-6578706c6f72"); // Custom UUID 
    const BLEUUID O2_UUID = BLEUUID("0000486a-1000-2000-3000-6578706c6f72");
    /*
     * Define the name of the characteristics as strings
     *      
     */
    const String PPO2_CHARACTERISTIC_DESCRIPTION = "Dioxygen Partial Pressure";
    const String O2_CHARACTERISTIC_DESCRIPTION = "Dioxygen Rate";

    /*
    *Format presentation descriptor
    *Définition des format de données pour le LOX02 pour le descripteur 0x2904 conforme à la norme BLE GATT
    */
    static uint8_t presentationTemp[7];
    static uint8_t presentationPPO2[7];
    static uint8_t presentationPressure[7];
    static uint8_t presentationO2[7];
  public :
    String getName(){return SensorName;};
    void powerOn();
    void initSensor();
    bool getData();
    void configEnvService(BLEService* pEnvService);
    void printSerialHeader();
    void printSerialData();     
    void setBLEData();   
};
