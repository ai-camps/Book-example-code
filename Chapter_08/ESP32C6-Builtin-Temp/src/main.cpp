// **********************************
// Code Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-21
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
// Security Considerations:
// This code does not involve external communication, minimizing security risks.
// However, ensure secure programming practices are followed in further development.
//
// Testing and Validation Approach:
// Test the code across various temperatures to ensure accurate sensor readings and correct LED feedback.

// Import necessary libraries
#include <Arduino.h>                    // Core Arduino library for basic functions and types
#include "builtin_temperature_sensor.h" // Library for accessing the ESP32-C6's built-in temperature sensor
#include <Adafruit_NeoPixel.h>          // Library for controlling RGB LEDs, including the built-in one on ESP32-C6

// Define constants for LED control
#define LED_PIN 8     // GPIO pin number where the built-in RGB LED is connected
#define STRIP_COUNT 1 // Number of strip (just one for built-in RGB LED on ESP32-C6)

// Global variables
int pixelIndex = 0;                                                  // Index of the LED to control (only one in this case, so it's set to 0)
Adafruit_NeoPixel strip(STRIP_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); // Initialize the NeoPixel strip object

// Color definitions for different temperature states
struct Colors
{
    static const uint32_t redColor = 0xFF0000;   // Color value for red
    static const uint32_t greenColor = 0x00FF00; // Color value for green
    static const uint32_t blueColor = 0x0000FF;  // Color value for blue
    static const uint32_t offColor = 0x000000;   // Color value for LED off (black/no color)
};

// Enumeration to represent different temperature states
enum TemperatureState
{
    Normal, // State for normal temperature range
    Low,    // State for low temperature
    High,   // State for high temperature
    Error   // State for sensor error indication
};

// Global variables for temperature thresholds
float lowTempThreshold = 15.0;  // Threshold for considering temperature as low
float highTempThreshold = 25.0; // Threshold for considering temperature as high

// Class to encapsulate temperature sensing and LED indication logic
class TemperatureIndicator
{
private:
    temperature_sensor_handle_t tempSensor;     // Handle to the built-in temperature sensor
    const unsigned long readInterval = 5000;    // Interval between temperature readings in milliseconds
    unsigned long previousMillis = 0;           // Timestamp of the last temperature reading
    TemperatureState tempState = Normal;        // Current temperature state
    unsigned long lastBlinkMillis = 0;          // Timestamp of the last LED blink
    const unsigned long redBlinkInterval = 100; // Interval for red LED blinking in milliseconds
    float lowThreshold;                         // Custom low temperature threshold
    float highThreshold;                        // Custom high temperature threshold

    static const uint32_t colors[4]; // Array to store color values for different states

    // Method to update the LED color based on the current temperature state
    void updateLED(TemperatureState state)
    {
        uint32_t color = colors[state]; // Select the appropriate color
        strip.setPixelColor(pixelIndex, color);  // Set the LED to the selected color
        strip.show();                   // Apply the color change
    }

public:
    // Constructor with custom temperature thresholds
    TemperatureIndicator(float lowThreshold, float highThreshold)
        : lowThreshold(lowThreshold), highThreshold(highThreshold) {}

    // Method to initialize the temperature sensor
    void initTemperatureSensor()
    {
        temperature_sensor_config_t config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 50);
        esp_err_t result = temperature_sensor_install(&config, &tempSensor);
        if (result == ESP_OK)
        {
            temperature_sensor_enable(tempSensor);
        }
        else
        {
            Serial.println("Error: Temperature sensor initialization failed");
            tempState = Error;
        }
    }

    // Method to periodically check the temperature and update the LED accordingly
    void checkTemperature()
    {
        unsigned long currentMillis = millis(); // Get the current time
        if (currentMillis - previousMillis >= readInterval)
        {                                                   // If it's time for another reading
            previousMillis = currentMillis;                 // Update the last reading timestamp
            float temperatureC = readTemperature();         // Read the temperature in Celsius
            float temperatureF = temperatureC * 9 / 5 + 32; // Convert to Fahrenheit

            // Print the temperature readings
            Serial.print("Temperature: ");
            Serial.print(temperatureC);
            Serial.print(" °C, ");
            Serial.print(temperatureF);
            Serial.print(" °F, LED Color State: ");

            // Update the temperature state and LED based on the reading
            if (!isnan(temperatureC))
            {
                if (temperatureC < lowThreshold)
                {
                    tempState = Low;
                    Serial.println("Cold (Blue)");
                }
                else if (temperatureC > highThreshold)
                {
                    tempState = High;
                    Serial.println("Hot (Red)");
                }
                else
                {
                    tempState = Normal;
                    Serial.println("Normal (Green)");
                }
                updateLED(tempState);
            }
        }

        // Blink the LED if the temperature is high
        if (tempState == High)
        {
            blinkLED(currentMillis);
        }
    }

    // Method to read the temperature from the sensor
    float readTemperature()
    {
        float temperature = 0.0;
        if (temperature_sensor_get_celsius(tempSensor, &temperature) != ESP_OK)
        {
            Serial.println("Error: Failed to read temperature");
            tempState = Error;
            updateLED(Error);
            return NAN;
        }
        return temperature;
    }

    // Method to blink the LED between red and off for high temperature indication
    void blinkLED(unsigned long currentMillis)
    {
        if (currentMillis - lastBlinkMillis >= redBlinkInterval)
        {                                    // If it's time to toggle the LED
            lastBlinkMillis = currentMillis; // Update the last blink timestamp
            static bool ledState = false;
            ledState = !ledState; // Toggle the LED state
            // Set the LED color directly to either red or off
            strip.setPixelColor(pixelIndex, ledState ? Colors::redColor : Colors::offColor);
            strip.show();
        }
    }
};

// Initialize the static array with color values for different temperature states
const uint32_t TemperatureIndicator::colors[4] = {
    Colors::greenColor, Colors::blueColor, Colors::redColor, Colors::offColor};

// Instantiate the TemperatureIndicator class with custom low and high temperature thresholds
TemperatureIndicator tempIndicator(lowTempThreshold, highTempThreshold);

void setup()
{
    Serial.begin(115200);                  // Start serial communication
    strip.begin();                         // Initialize the NeoPixel strip
    strip.show();                          // Turn off all pixels initially
    tempIndicator.initTemperatureSensor(); // Initialize the temperature sensor
}

void loop()
{
    tempIndicator.checkTemperature(); // Periodically check the temperature and update the LED
}
