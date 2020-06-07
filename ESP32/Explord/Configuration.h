#pragma once

/*
 * Define the device settings : name, number, ...
 */

#define DEVICE_NAME "Explord-" //Name of the device
#define DEVICE_NUMBER "02" //Number of the device

#define READ_DELAY 1 //Initial read delay in seconds

#define BLINK_TIME 30 //Time for the led to blink in ms

#define DEVICE_MAC {0x80,0x7D,0x3A,0x00,0x00,0x00} //Initial mac adress : fourth and fifth bytes changed according to sensor plugged and to device number

#define DEFAULT_LOG_FILE "/explord_data.csv" //Default log file name for the csv log

/*
 * Define the pins for the ESP TTGO T1
 */

#define LEDPIN 22 //Built In LED
#define PPPOWERPIN 21 //Power the resistor bridge to detect plugged sensor
#define PPDETECTPIN 32 //Analog read pin to detect plugged sensor : A4 is for gpio32, ADC1 is used with channel 4

#define ENPIN 25 //Enable sensor pin : connected to low switch mosfet that cut ground
#define DATA1PIN 18 //RX, SDA, OneWire
#define DATA2PIN 19 //TX, SCL

#define PIN_MISO 2 //Pins for the built-in SD card reader
#define PIN_MOSI 15
#define PIN_CLK 14
#define PIN_CS 13
