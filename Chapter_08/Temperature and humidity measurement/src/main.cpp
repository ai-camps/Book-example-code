// **********************************
// Code Explanation
// **********************************
// Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-14
//
// Code Purpose:
// This code is designed to read temperature in both Fahrenheit and Celsius from a DHT11 sensor 
// connected to an ESP32-C6 and print these values to the serial monitor.
//
// Hardware Connection:
// DHT11 VCC to ESP32-C6 3.3V (Pin 1)
// DHT11 GND to ESP32-C6 GND (Pin 15, 18, 21, or 32)
// DHT11 DATA to ESP32-C6 GPIO4 (Pin 3)
//
// Security Considerations:
// Secure handling of data is crucial in IoT devices. Ensure that data transmission over networks is encrypted.
//
// Testing and Validation:
// Test the setup in various environmental conditions to ensure accurate readings. Validate the sensor's response
// by comparing its readings with a trusted thermometer.
//
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>
#include "DHT.h"

// **********************************
// Constants Declaration
// **********************************
#define DHTPIN 4                        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11                   // DHT 11 sensor type
#define FIRMWARE_AUTHOR "ESP32-C6 Coding Assistant" // Author of the firmware

// **********************************
// Variables Declaration
// **********************************
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for temperature and humidity readings

// **********************************
// Functions Definition
// **********************************
// No additional functions are required for this basic example

// **********************************
// Setup() Function
// **********************************
void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud rate
  dht.begin();          // Initialize the DHT sensor for operation
}

// **********************************
// Loop() Function
// **********************************
void loop() {
  // Wait a few seconds between measurements
  delay(2000); 

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();        // Read humidity (percentage)
  float t = dht.readTemperature();     // Read temperature as Celsius
  float f = dht.readTemperature(true); // Read temperature as Fahrenheit

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index
  // The heat index is a measure of how hot it really feels when relative humidity is factored in with the actual air temperature.
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  // Print the results to the Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C ");
  Serial.print(f);
  Serial.print(" °F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" °C ");
  Serial.print(hif);
  Serial.println(" °F");
}
