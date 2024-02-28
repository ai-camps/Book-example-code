#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  // Give some time for the serial monitor to start
  delay(1000);
}

void loop() {
  Serial.println("\nESP32 Chip information:");

  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());

  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());

  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());

  Serial.print("Number of Cores: ");
  Serial.println(ESP.getChipCores());

  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");

  Serial.print("Max CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");

  Serial.print("Flash Chip Size: ");
  Serial.print(ESP.getFlashChipSize());
  Serial.println(" bytes");

  Serial.print("Flash Chip Speed: ");
  Serial.print(ESP.getFlashChipSpeed());
  Serial.println(" Hz");

  Serial.print("PSRAM Size: ");
  Serial.print(ESP.getPsramSize());
  Serial.println(" bytes");

  Serial.print("Free PSRAM: ");
  Serial.print(ESP.getFreePsram());
  Serial.println(" bytes");

  Serial.print("Sketch Size: ");
  Serial.print(ESP.getSketchSize());
  Serial.println(" bytes");

  Serial.print("Free Sketch Space: ");
  Serial.print(ESP.getFreeSketchSpace());
  Serial.println(" bytes");

  Serial.print("Heap Size: ");
  Serial.print(ESP.getHeapSize());
  Serial.println(" bytes");

  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");

  Serial.print("Min Free Heap: ");
  Serial.print(ESP.getMinFreeHeap());
  Serial.println(" bytes");

  Serial.print("Max Alloc Heap: ");
  Serial.print(ESP.getMaxAllocHeap());
  Serial.println(" bytes");

  FlashMode_t flashMode = ESP.getFlashChipMode();
  Serial.print("Flash Chip Mode: ");
  Serial.println(flashMode == FM_QIO ? "QIO" : flashMode == FM_QOUT ? "QOUT" : flashMode == FM_DIO ? "DIO" : "DOUT");

  // Print the MAC address of the ESP32
  Serial.print("WiFi MAC Address: ");
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

#ifdef ESP32
  float tempF = temperatureRead(); // Assume temperatureRead() returns temperature in Fahrenheit
  float tempC = (tempF - 32) * 5.0 / 9.0; // Convert Fahrenheit to Celsius

  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.print(" Celsius, ");
  Serial.print(tempF);
  Serial.println(" Fahrenheit");
#endif


  // Add a delay to prevent flooding the serial output.
  delay(10000); // Wait for 10 seconds before the next printout
}
