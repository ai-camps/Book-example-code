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
// 2. Indicate sensor read error with a steady red RGB LED.
// 3. Show a steady green RGB LED for temperature range 15°C to 25°C and humidity range 10%RH to 60%RH.
// 4. Show a steady blue RGB LED for temperature below 15°C or humidity below 10%RH.
// 5. Indicate high temperature (>25°C) or high humidity (>60%RH) with a red blinking RGB LED at 1000ms intervals.
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
//
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "DHT.h"

// **********************************
// Constants Declaration
// **********************************
#define DHTPIN 4                                   // DHT11 data pin
#define DHTTYPE DHT11                              // Sensor type
#define LED_PIN 8                                  // WS2812B data pin
#define NUM_LEDS 1                                 // Number of LEDs in the strip
#define READ_INTERVAL 3000                         // Sensor read interval in milliseconds
#define BLINK_INTERVAL 1000                          // Blink interval for errors/high conditions

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
DHT dht(DHTPIN, DHTTYPE);

// **********************************
// Variables Declaration
// **********************************
unsigned long lastReadTime = 0;
unsigned long lastBlinkTime = 0;
bool isHighCondition = false;                      // For high condition blinking

// **********************************
// Functions Definition
// **********************************
void displayColor(uint8_t red, uint8_t green, uint8_t blue) {
    strip.setPixelColor(0, strip.Color(red, green, blue));
    strip.show();
}

// Function to convert Celsius to Fahrenheit
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
    unsigned long currentMillis = millis();

    // Handle sensor reading every READ_INTERVAL
    if (currentMillis - lastReadTime > READ_INTERVAL) {
        lastReadTime = currentMillis;
        float humidity = dht.readHumidity();
        float tempC = dht.readTemperature();
        float tempF = convertToFahrenheit(tempC); // Convert to Fahrenheit

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%, Temperature: ");
        Serial.print(tempC);
        Serial.print("C ");
        Serial.print(tempF);
        Serial.println("F");

        if (isnan(humidity) || isnan(tempC)) {
            displayColor(255, 0, 0); // Steady red for sensor error
            isHighCondition = false;
        } else if ((tempC >= 15 && tempC <= 25) && (humidity >= 10 && humidity <= 60)) {
            displayColor(0, 255, 0); // Steady green for normal condition
            isHighCondition = false;
        } else if (tempC < 15 || humidity < 10) {
            displayColor(0, 0, 255); // Steady blue for low condition
            isHighCondition = false;
        } else {
            isHighCondition = true;
            lastBlinkTime = currentMillis; // Reset blink timing
        }
    }

    // Handle blinking for high condition outside the main if condition to ensure continuous blinking
    if (isHighCondition && (currentMillis - lastBlinkTime > BLINK_INTERVAL)) {
        lastBlinkTime = currentMillis;
        strip.setPixelColor(0, strip.Color(255, 0, 0));
        strip.show();
        delay(10); // Maintain red for a short period to ensure visibility
        strip.clear(); // Turn off LED
        strip.show();
    }
}
