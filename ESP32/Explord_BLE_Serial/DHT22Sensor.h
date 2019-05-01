#pragma once

#include "Characteristic.h"
#include <DHTesp.h>
  
class DHT22Sensor {
  private :
    DHTesp dht;
  public :
    void initSensor();
    bool getData(); 
};
