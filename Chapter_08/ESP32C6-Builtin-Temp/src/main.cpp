// **********************************
// Code Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-18
// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This code snippet is designed to measure the temperature using the ESP32-C6's built-in temperature sensor
// and visually indicate the temperature range using the built-in RGB LED. 
//
// Requirement:
// 1. Use the ESP32-C6's built-in RGB LED for temperature range indication.
// 2. Periodically read and print the temperature in Celsius and Fahrenheit.
// 3. Indicate sensor errors with a steady red LED.
// 4. Indicate normal, low, and high temperatures with green, blue, and blinking red LEDs respectively.
//
// Hardware Connection:
// The built-in temperature sensor and RGB LED on the ESP32-C6 are used, requiring no external hardware connections.
//
// New Created Functions:
// initTemperatureSensor() - Initializes the temperature sensor.
// readTemperatureSensor() - Reads the temperature from the sensor.
// setLEDColor() - Sets the RGB LED color.
// handleTemperature() - Determines the LED color based on the temperature.
// 
// Security Considerations:
// This code does not involve external communication, minimizing security risks.
// However, ensure secure programming practices are followed in further development.
//
// Testing and Validation Approach:
// Test the code across various temperatures to ensure accurate sensor readings and correct LED feedback.
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>                    // Import the Arduino framework
#include "builtin_temperature_sensor.h" // Include the header for the built-in temperature sensor
#include <Adafruit_NeoPixel.h>          // Import the Adafruit NeoPixel library for controlling the built-in RGB LED

// **********************************
// Constants Declaration
// **********************************
#define ledStrip 1                                                // LED strip count, as ESP32-C6 has one built-in RGB LED
#define ledGpio 8                                                 // LED GPIO PIN, which is GPIO8 for ESP32-C6
int pixelIndex = 0;                                               // Index of the LED you want to control
Adafruit_NeoPixel strip(ledStrip, ledGpio, NEO_GRB + NEO_KHZ800); // Initialize Adafruit NeoPixel strip

// GRB (Not RGB !) LED color definitions
#define redColor strip.Color(255, 0, 0)   // Red color for error or high temperature
#define greenColor strip.Color(0, 255, 0) // Green color for normal temperature
#define blueColor strip.Color(0, 0, 255)  // Blue color for low temperature
#define offColor strip.Color(0, 0, 0)     // Off color

// Temperature thresholds
const float lowTempThreshold = 10.0;  // Low temperature threshold in Celsius
const float highTempThreshold = 15.0; // High temperature threshold in Celsius

// Timing constants
const unsigned long readInterval = 5000;    // Interval to read the temperature sensor in milliseconds
const unsigned long blinkInterval = 100;   // Blinking interval for high temperature indication in milliseconds

// **********************************
// Variables Declaration
// **********************************
static temperature_sensor_handle_t tsens; // Handle for the temperature sensor
unsigned long previousMillis = 0; // Store the previous time
unsigned long lastBlinkTime = 0;  // Store the last time the LED was blinked
unsigned long lastReadTime = 0;   // Store the last time the temperature was read
bool ledState = false;            // Store the current state of the LED (on or off)
bool isHighTemperature = false;   // Store the current temperature state (high or low)

// **********************************
// Functions Declaration
// **********************************

// Function: initTemperatureSensor
// Description: Initializes the built-in temperature sensor.
// Parameters: None
// Returns: void
void initTemperatureSensor() {
    temperature_sensor_config_t tsens_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
    esp_err_t result = temperature_sensor_install(&tsens_config, &tsens);
    if (result != ESP_OK) {
        Serial.println("Failed to initialize temperature sensor");
    } else {
        result = temperature_sensor_enable(tsens);
        if (result != ESP_OK) {
            Serial.println("Failed to enable temperature sensor");
        }
    }
}

// Function: readTemperatureSensor
// Description: Reads the temperature from the built-in temperature sensor.
// Parameters: None
// Returns: float - The temperature in Celsius, or NAN if read fails.
float readTemperatureSensor() {
    float temperature = 0.0;
    esp_err_t result = temperature_sensor_get_celsius(tsens, &temperature);
    if (result != ESP_OK) {
        Serial.println("Failed to read temperature");
        return NAN;
    }
    return temperature;
}

// Function: setLEDColor
// Description: Sets the color of the built-in RGB LED.
// Parameters:
// - color: The color to set the LED to.
// - colorName: The name of the color (for debugging).
// Returns: void
void setLEDColor(uint32_t color, const char* colorName) {
    strip.setPixelColor(0, color);
    strip.show();
    Serial.print("LED set to ");
    Serial.println(colorName);
}

// Function: handleBlinking
// Description: Handles the blinking of the built-in RGB LED.
// Parameters: None
// Returns: void
// Note: This function handles the temperature state based on the current temperature value.
// If the temperature is below the low temperature threshold, it sets the LED strip to blue.
// If the temperature is between the low and high temperature thresholds, it sets the LED strip to green.
// If the temperature is above the high temperature threshold, it sets the LED strip to blinking red.
// If the temperature reading is invalid, it sets the LED strip to red and prints an error message to the serial monitor.
void handleBlinking() {
    static bool ledState = false;
    if (millis() - lastBlinkTime > blinkInterval) {
        lastBlinkTime = millis();
        ledState = !ledState;
        setLEDColor(ledState ? redColor : offColor, ledState ? "Red Blink" : "Off");
    }
}

// **********************************
// Setup Function
// **********************************

void setup() {
    Serial.begin(115200); // Initialize serial communication
    strip.begin();        // Initialize the NeoPixel strip
    strip.show();         // Turn off all pixels initially
    initTemperatureSensor(); // Initialize the temperature sensor
}

// **********************************
// Main loop
// **********************************
// Function: loop
// Description: Main loop function for repeating tasks.
// Parameters: void
// Returns: void
// Note: This function reads the temperature from the built-in temperature sensor every 2 seconds.
// It updates the NeoPixel color based on the temperature value.
// If the temperature is below 10 Celsius, it displays blue.
// If the temperature is between 10 and 25 Celsius, it displays green.
// If the temperature is above 25 Celsius, it displays a blinking red.
// If the temperature reading is invalid, it displays a red color and prints an error message to the serial monitor.
// The blinking red is handled by the BlinkRedLED object.
// The blink() function automatically handles the blinking.
void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastReadTime >= readInterval) {
        lastReadTime = currentMillis;
        float temperature = readTemperatureSensor();

        if (isnan(temperature)) {
            Serial.println("No sensor found - LED set to Red");
            setLEDColor(redColor, "Red");
            isHighTemperature = false;
        } else {
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.print(" °C, ");
            Serial.print(temperature * 9.0 / 5.0 + 32.0);
            Serial.println(" °F");

            isHighTemperature = temperature > highTempThreshold;

            if (!isHighTemperature) {
                if (temperature < lowTempThreshold) {
                    setLEDColor(blueColor, "Blue");
                } else {
                    setLEDColor(greenColor, "Green");
                }
            }
        }
    }

    if (isHighTemperature) {
        handleBlinking();
    } else {
        static bool firstTime = true;
        if (firstTime) {
            setLEDColor(offColor, "Off");
            firstTime = false;
        }
    }
}
