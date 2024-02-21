// ESP32Info.h
#ifndef ESP32INFO_H
#define ESP32INFO_H

#include <Arduino.h>
#include <WiFi.h>

class ESP32Info {
public:
    static void initializeSerial();
    static void printChipInfo();
};

#endif // ESP32INFO_H
