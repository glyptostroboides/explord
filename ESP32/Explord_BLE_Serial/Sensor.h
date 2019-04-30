#pragma once

#include "BLE_Char.h"
#include "DHT22Sensor.h"
#include "MHZ16Sensor.h"
#include "LOX02Sensor.h"

const int Max_Characteristic = 4;

class Sensor {
  private:
    int SensorType;
    BLEChar *_Char_Set[Max_Characteristic];
  public:
    void begin(int SensorType);
};
