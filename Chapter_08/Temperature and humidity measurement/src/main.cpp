// Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-15
// **********************************
// Code Explanation
// **********************************
// 
// Code Purpose:
// This code interfaces an ESP32-C6 with a DHT11 sensor and a WS2812B RGB LED.
// It reads temperature and humidity data every 3 seconds, and uses the RGB LED
// to provide visual feedback on the environmental conditions or sensor errors.
//
// Requirements:
// 1. Read temperature (0°C to 50°C) and humidity (20% to 90%RH) data every 3 seconds.
// 2. Indicate sensor read error with a red blinking RGB LED at 100ms intervals.
// 3. Show a steady green RGB LED for temperature range 15°C to 25°C and humidity range 10%RH to 60%RH.
// 4. Show a steady blue RGB LED for temperature below 15°C or humidity below 10%RH.
// 5. Show a steady red RGB LED for temperature above 25°C or humidity above 60%RH.
//
// Hardware Connection:
// DHT11 Data Pin -> GPIO4
// WS2812B RGB LED Data Pin -> GPIO8 (refer to ESP32-C6-Pinout.txt for correct pin)
//
// New Created Functions:
// - convertToFahrenheit(): Converts Celsius to Fahrenheit.
// - displayColor(): Controls the WS2812B RGB LED color.
//
// Security Considerations:
// Handle the DHT11 and WS2812B with care to prevent electrostatic discharges.
//
// Testing and Validation Approach:
// Test by monitoring the serial output and observing the RGB LED color changes under
// different temperature and humidity conditions.
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "DHT.h"

// **********************************
// Constants Declaration
// **********************************
#define FIRMWARE_AUTHOR "ESP32-C6 Coding Assistant"
#define DHTPIN 4 // DHT11 data pin
#define DHTTYPE DHT11
#define LED_PIN 8 // WS2812B data pin
#define NUM_LEDS 1 // Number of LEDs in the strip
#define READ_INTERVAL 3000 // Sensor read interval in milliseconds

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
DHT dht(DHTPIN, DHTTYPE);

// **********************************
// Variables Declaration
// **********************************
unsigned long lastReadTime = 0;

// **********************************
// Functions Definition
// **********************************
void displayColor(uint8_t red, uint8_t green, uint8_t blue) {
    strip.setPixelColor(0, strip.Color(red, green, blue));
    strip.show();
}

float convertToFahrenheit(float celsius) {
    return celsius * 9.0 / 5.0 + 32;
}

// **********************************
// Setup() Function
// **********************************
void setup() {
    Serial.begin(115200);
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    dht.begin();
}

// **********************************
// Loop() Function
// **********************************
void loop() {
    if (millis() - lastReadTime > READ_INTERVAL) {
        lastReadTime = millis();
        float humidity = dht.readHumidity();
        float tempC = dht.readTemperature();
        float tempF = convertToFahrenheit(tempC);

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%, Temp: ");
        Serial.print(tempC);
        Serial.print("C ");
        Serial.print(tempF);
        Serial.println("F");

        if (isnan(humidity) || isnan(tempC)) {
            // Blink red for sensor error
            displayColor(255, 0, 0); // Red
            delay(100);
            displayColor(0, 0, 0); // Off
            delay(100);
        } else if ((tempC >= 15 && tempC <= 25) && (humidity >= 10 && humidity <= 60)) {
            // Steady green for normal condition
            displayColor(0, 255, 0); // Green
        } else if (tempC < 15 || humidity < 10) {
            // Steady blue for low condition
            displayColor(0, 0, 255); // Blue
        } else if (tempC > 25 || humidity > 60) {
            // Steady red for high condition
            displayColor(255, 0, 0); // Red
        }
    }
}
