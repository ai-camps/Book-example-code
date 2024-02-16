#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  // Give some time for the serial monitor to start
  delay(1000);

  Serial.println("ESP32 Chip information:");

  // Print the CPU model
  Serial.print("Model: ");
  Serial.println(ESP.getChipModel());

  // Print the CPU revision
  Serial.print("Revision: ");
  Serial.println(ESP.getChipRevision());

  // Print the number of cores
  Serial.print("Number of Cores: ");
  Serial.println(ESP.getChipCores());

  // Print the CPU frequency
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");

  // Print the size of flash memory
  Serial.print("Flash Chip Size: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");

  // Print the speed of the flash memory
  Serial.print("Flash Chip Speed: ");
  Serial.print(ESP.getFlashChipSpeed());
  Serial.println(" Hz");

  // Print the size of the sketch
  Serial.print("Sketch Size: ");
  Serial.print(ESP.getSketchSize());
  Serial.println(" bytes");

  // Print the amount of free space on the flash memory
  Serial.print("Free Sketch Space: ");
  Serial.print(ESP.getFreeSketchSpace());
  Serial.println(" bytes");

  // Print the amount of free heap memory (in bytes)
  Serial.print("Free Heap Size: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");

  // Print the MAC address of the ESP32
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Print the eFuse MAC address
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

  // Print the SDK Version
  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());

  // Flash Mode
  FlashMode_t ideMode = ESP.getFlashChipMode();
  Serial.print("Flash Chip Mode: ");
  Serial.println((ideMode == FM_QIO) ? "QIO" : (ideMode == FM_QOUT) ? "QOUT" : (ideMode == FM_DIO) ? "DIO" : "DOUT");

}
void loop() {
  // put your main code here, to run repeatedly:
}
