// **********************************
// Code Explanation
// **********************************
// Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-14
//
// Code Purpose:
// This code is designed to interface an ESP32-C6 with a DHT11 temperature and humidity sensor.
// It reads the temperature in Fahrenheit and prints the value to the serial monitor every minute.
// The Adafruit Unified Sensor and DHT sensor libraries are used to facilitate communication with the sensor.
// 
// Hardware Connection:
// DHT11 VCC to ESP32-C6 3.3V (Pin 1)
// DHT11 GND to ESP32-C6 GND (Pin 15, 18, 21, or 32)
// DHT11 DATA to ESP32-C6 GPIO4 (Pin 3)
//
// Security Considerations:
// Ensure secure handling of serial output in your application environment.
// 
// Testing and Validation:
// Test the sensor's response in a controlled environment to validate accuracy.
// 
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>
#include "DHT.h"

// **********************************
// Variables Declaration
// **********************************
#define DHTPIN 4      // Digital pin connected to the DHT sensor, using IO4 as per ESP32-C6 pinout.
#define DHTTYPE DHT11 // DHT 11 sensor type

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

// **********************************
// Functions Definition
// **********************************
// No additional functions required for this basic example

// **********************************
// Setup() Function
// **********************************
void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud rate
  dht.begin();          // Initialize the DHT sensor
}

// **********************************
// Loop() Function
// **********************************
void loop() {
  // Wait a few seconds between measurements (delay for 1 minute)
  delay(60000); 

  // Reading temperature in Fahrenheit
  float f = dht.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print the temperature in Fahrenheit to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(f);
  Serial.println(" °F");
}