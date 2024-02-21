// ESP32Info.cpp
#include "ESP32Info.h"

void ESP32Info::initializeSerial() {
    Serial.begin(115200);
    delay(1000); // Wait for serial monitor to initialize
}

void ESP32Info::printChipInfo() {
    Serial.println("ESP32 Chip information:");

    Serial.print("Model: ");
    Serial.println(ESP.getChipModel());

    Serial.print("Revision: ");
    Serial.println(ESP.getChipRevision());

    Serial.print("Number of Cores: ");
    Serial.println(ESP.getChipCores());

    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");

    Serial.print("Flash Chip Size: ");
    Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
    Serial.println(" MB");

    Serial.print("Flash Chip Speed: ");
    Serial.print(ESP.getFlashChipSpeed());
    Serial.println(" Hz");

    Serial.print("Sketch Size: ");
    Serial.print(ESP.getSketchSize());
    Serial.println(" bytes");

    Serial.print("Free Sketch Space: ");
    Serial.print(ESP.getFreeSketchSpace());
    Serial.println(" bytes");

    Serial.print("Free Heap Size: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    uint64_t chipid = ESP.getEfuseMac(); // Get MAC address from eFuse
    uint8_t mac[6];
    mac[0] = (chipid >> 40) & 0xFF;
    mac[1] = (chipid >> 32) & 0xFF;
    mac[2] = (chipid >> 24) & 0xFF;
    mac[3] = (chipid >> 16) & 0xFF;
    mac[4] = (chipid >> 8) & 0xFF;
    mac[5] = chipid & 0xFF;
    char macStr[18] = {0};
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.print("eFuse MAC: ");
    Serial.println(macStr);

    Serial.print("SDK Version: ");
    Serial.println(ESP.getSdkVersion());

    FlashMode_t ideMode = ESP.getFlashChipMode();
    Serial.print("Flash Chip Mode: ");
    Serial.println((ideMode == FM_QIO) ? "QIO" : (ideMode == FM_QOUT) ? "QOUT" : (ideMode == FM_DIO) ? "DIO" : "DOUT");
}
