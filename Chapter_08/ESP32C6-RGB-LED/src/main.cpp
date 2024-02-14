// **********************************
// Code Explanation
// **********************************
// Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-14
//
// Code Purpose:
// This code is designed to control an RGB LED connected to an ESP32-C6, making it blink
// in Red, Green, and Blue colors sequentially. It utilizes the Adafruit NeoPixel library
// for easy control of the LED.
//
// Hardware Connection:
// RGB LED data pin to ESP32-C6 GPIO8 (Pin 9 as per ESP32-C6 Pinout)
//
// Security Considerations:
// As this project involves basic LED control, the primary security consideration is
// ensuring physical safety when connecting the LED and handling the ESP32-C6.
//
// Testing and Validation:
// Verify correct LED operation by observing the sequential color changes. Ensure there
// are no short circuits, and the LED is correctly connected to prevent damage to the ESP32-C6.
//
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// **********************************
// Constants Declaration
// **********************************
#define FIRMWARE_AUTHOR "ESP32-C6 Coding Assistant" // Firmware author name
#define PIN 8 // Pin number for the RGB LED, using GPIO8 as per ESP32-C6 pinout
#define NUMPIXELS 1 // Number of pixels in the NeoPixel (just one in this case)

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Define colors as presets (Red, Green, Blue)
const uint32_t RED = pixels.Color(255, 0, 0); // Red color
const uint32_t GREEN = pixels.Color(0, 255, 0); // Green color
const uint32_t BLUE = pixels.Color(0, 0, 255); // Blue color

// **********************************
// Variables Declaration
// **********************************
// No additional global variables required for this basic example

// **********************************
// Functions Definition
// **********************************
// No additional functions required for this basic example

// **********************************
// Setup() Function
// **********************************
void setup() {
  pixels.begin(); // Initialize the NeoPixel library.
}

// **********************************
// Loop() Function
// **********************************
void loop() {
  // Set the pixel (LED) to red
  pixels.setPixelColor(0, RED); // Pixel number, Color
  pixels.show(); // Update the actual LED
  delay(1000); // Wait for a second

  // Set the pixel (LED) to green
  pixels.setPixelColor(0, GREEN);
  pixels.show();
  delay(1000); // Wait for a second

  // Set the pixel (LED) to blue
  pixels.setPixelColor(0, BLUE);
  pixels.show();
  delay(1000); // Wait for a second
}
