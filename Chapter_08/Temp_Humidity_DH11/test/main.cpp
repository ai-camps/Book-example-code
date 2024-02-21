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
#include <Arduino.h>           // Includes the main Arduino library for general IO functions.
#include <Adafruit_NeoPixel.h> // Incorporates the Adafruit NeoPixel library for controlling the WS2812B RGB LED.
#include "DHT.h"               // Includes the DHT sensor library for interfacing with the DHT11 sensor.

// **********************************
// Constants Declaration
// **********************************
#define dhtPin 4      // Pin connected to the DHT11 data line
#define dhtType DHT11 // Specify using a DHT11 sensor

// ESP32-C6 has a built-in RGB LED, which is connected to GPIO8, and driven d by the Adafruit NeoPixel library
#define ledStrip 1  // Led strip count
#define ledGpio 8   // Led GPIO pin
int pixelIndex = 0; // Index of the LED you want to control

// Initialize Adafruit NeoPixel strip
Adafruit_NeoPixel strip(ledStrip, ledGpio, NEO_GRB + NEO_KHZ800); 

// RGB LED color definitions
#define redColor strip.Color(255, 0, 0)   // Red color
#define greenColor strip.Color(0, 255, 0) // Green color
#define blueColor strip.Color(0, 0, 255)  // Blue color
#define offColor strip.Color(0, 0, 0)     // Off color

DHT dht(dhtPin, dhtType); // Initialize DHT sensor object

// **********************************
// Variables Declaration
// **********************************

// ESP32-C6 has a built-in temperature sensor, which is used to measure the ambient temperature, and driven by "builtin_temperature_sensor" library
// Temperature thresholds
const float lowTempThreshold = 10.0;  // Low temperature threshold
const float highTempThreshold = 25.0; // High temperature threshold

// Humidity thresholds
const float lowHumThreshold = 10.0;  // Low humidity threshold
const float highHumThreshold = 80.0; // High humidity threshold

// Time interval for reading the temperature sensor in milliseconds
const unsigned long readInterval = 5000;

// Time interval for red blinking in milliseconds
const unsigned long blinkInterval = 100;

// Sensor read and LED blink timing variables
unsigned long lastReadTime = 0;  // Time of the last sensor read
bool highConditionFlag = false;  // Flag for high temp/humidity condition
unsigned long lastBlinkTime = 0; // Time of the last LED blink
bool ledState = false;           // Current LED state for blinking

// **********************************
// Functions Definition
// **********************************

// Function: setLEDColor
// Description: Set the color of the NeoPixel strip.
// Parameters:
// - color: The color to set the LED strip to.
// - colorName: The name of the color.
// Returns: void
// Note: This function sets the color of the NeoPixel strip to the specified color.
// It also prints the color name to the serial monitor.
// The color is specified as a 32-bit unsigned integer, where each byte represents a color channel.
// The red channel is represented by the first byte, the green channel by the second byte, and the blue channel by the third byte.
// The off color is represented by 0x000000 (black).
// The red color is represented by 0xFF0000 (red).
// The green color is represented by 0x00FF00 (green).
// The blue color is represented by 0x0000FF (blue).
void setLEDColor(uint32_t color, const char *colorName)
{
    strip.setPixelColor(pixelIndex, color);
    strip.show();
    Serial.print("LED set to ");
    Serial.println(colorName);
}

// Function: convertToFahrenheit
float convertToFahrenheit(float celsius) {
  return celsius * 9.0 / 5.0 + 32;
}

// Function: isNormalCondition
bool isNormalCondition(float temp, float humidity)
{
    return temp >= lowTempThreshold && temp <= highTempThreshold &&
           humidity >= lowHumThreshold && humidity <= highHumThreshold;
}

// Function: isColdOrDryCondition
bool isColdOrDryCondition(float temp, float humidity)
{
    return temp < lowTempThreshold || humidity < lowHumThreshold;
}

// Function: isHighCondition
bool isHighCondition(float temp, float humidity)
{
    return temp > highTempThreshold || humidity > highHumThreshold;
}

// Function: handleTemperature
// Description: Handle the temperature state.
// Parameters: void
// Returns: void
// Note: This function handles the temperature state based on the current temperature value.
// If the temperature is below the low temperature threshold, it sets the LED strip to blue.
// If the temperature is between the low and high temperature thresholds, it sets the LED strip to green.
// If the temperature is above the high temperature threshold, it sets the LED strip to blinking red.
// If the temperature reading is invalid, it sets the LED strip to red and prints an error message to the serial monitor.
void handleBlinking() {
    if (millis() - lastBlinkTime > blinkInterval) {
        lastBlinkTime = millis();
        ledState = !ledState;
        setLEDColor(ledState ? redColor : offColor, ledState ? "Red Blink" : "Off");
    }
}

// **********************************
// Setup
// **********************************
// Function: setup
// Description: Setup function for initialization.
// Parameters: void
// Returns: void
void setup()
{
    Serial.begin(115200); // Start serial communication for logging
    strip.begin();        // Initialize the LED strip
    strip.show();         // Turn off all LEDs at startup
    dht.begin();          // Initialize the DHT sensor
}

// **********************************
// Main loop
// **********************************
// Function: loop
// Description: Main loop function for reading sensor data and controlling the LED.
// Parameters: void
// Returns: void
void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastReadTime >= readInterval) {
        lastReadTime = currentMillis;
        float humidity = dht.readHumidity();
        float tempC = dht.readTemperature();

        if (isnan(humidity) || isnan(tempC)) {
            Serial.println("Failed to read from DHT sensor!");
            setLEDColor(redColor, "Red");
            strip.show();
            highConditionFlag = false;
        } else {
            Serial.printf("Humidity: %.1f%%, Temperature: %.1fC (%.1fF)\n", humidity, tempC, convertToFahrenheit(tempC));

            if (isNormalCondition(tempC, humidity)) {
                setLEDColor(greenColor, "Green");
                highConditionFlag = false;
            } else if (isColdOrDryCondition(tempC, humidity)) {
                setLEDColor(blueColor, "Blue");
                highConditionFlag = false;
            } else if (isHighCondition(tempC, humidity)) {
                highConditionFlag = true;
            }
        }
    }

    if (highConditionFlag) {
        handleBlinking();
    } else if (ledState) { 
        ledState = false;
        setLEDColor(offColor, "Off");
    }
}