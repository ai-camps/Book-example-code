// **********************************
// Code Explanation
// **********************************
// Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-14
//
// Code Purpose:
// This code is designed to read the built-in temperature sensor of the ESP32-C6
// and print the temperature readings in Celsius and Fahrenheit to the serial monitor.
//
// Hardware Connection:
// No external hardware connections are needed as this utilizes the ESP32-C6's built-in temperature sensor.
//
// Security Considerations:
// Ensure the serial output is securely managed in your application environment.
//
// Testing and Validation:
// Monitor the serial output for temperature readings. Test the sensor's functionality by comparing
// the readings with a known temperature source for rough validation, considering the sensor's accuracy.

// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>
#include "builtin_temperature_sensor.h"

// **********************************
// Constants Declaration
// **********************************
#define FIRMWARE_AUTHOR "ESP32-C6 Coding Assistant" // Firmware author name

// **********************************
// Variables Declaration
// **********************************
// No global variables required for this example

// **********************************
// Functions Definition
// **********************************
// Function prototype for initializing the temperature sensor
void initTemperatureSensor();

// Function prototype for reading the temperature sensor
float readTemperatureSensor();

// **********************************
// Setup() Function
// **********************************
void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud rate
  initTemperatureSensor(); // Initialize the built-in temperature sensor
}

// **********************************
// Loop() Function
// **********************************
void loop() {
  float temperatureC = readTemperatureSensor(); // Read temperature in Celsius
  float temperatureF = temperatureC * 9.0 / 5.0 + 32; // Convert to Fahrenheit
  
  // Print both Celsius and Fahrenheit temperatures to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.print(" °C, ");
  Serial.print(temperatureF);
  Serial.println(" °F");
  
  delay(2000); // Delay for 2 seconds before the next reading
}