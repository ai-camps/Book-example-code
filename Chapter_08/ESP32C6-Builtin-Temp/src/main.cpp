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
// Security Considerations:
// This code does not involve external communication, minimizing security risks.
// However, ensure secure programming practices are followed in further development.
//
// Testing and Validation Approach:
// Test the code across various temperatures to ensure accurate sensor readings and correct LED feedback.

#include <Arduino.h>                    // Core Arduino library for basic functions and types
#include "builtin_temperature_sensor.h" // Library for accessing the ESP32-C6's built-in temperature sensor
#include <Adafruit_NeoPixel.h>          // Library for controlling RGB LEDs, including the built-in one on ESP32-C6
#include <map>                          // Includes the map library for mapping alert types to their corresponding configurations.

// Define constants for LED control
#define LED_PIN 8     // Defines the pin number connected to the NeoPixel LED.
#define STRIP_COUNT 1 // Defines the number of the strip
#define BUZZER_PIN 10 // Defines the pin number connected to the buzzer.

int pixelIndex = 0;                       // Index of the LED to control (only one in this case, so it's set to 0)
static temperature_sensor_handle_t tsens; // Temperature sensor handle
float lowTempThreshold = 10.0;            // Threshold for considering temperature as low
float highTempThreshold = 20.0;           // Threshold for considering temperature as high
unsigned long readInterval = 5000;        // Interval for reading temperature
unsigned long lastReadTime = 0;           // Timestamp of the last temperature reading
unsigned long blinkInterval = 100;        // Interval for LED blinking
unsigned long lastBlinkTime = 0;          // tracking the last blink time
bool ledBlinkState = false;               // blinking state tracking

// Class to control the LED for visual alerts
class LEDController
{
private:
    Adafruit_NeoPixel strip;         // NeoPixel strip object
    bool ledState = false;           // Tracks the current LED state (on/off) for blinking
    unsigned long lastBlinkTime = 0; // Timestamp of the last blink action

public:
    enum class Colors // Enum for LED colors
    {
        Red = 0xFF0000,   // Color value for red
        Green = 0x00FF00, // Color value for green
        Blue = 0x0000FF,  // Color value for blue
        Off = 0x000000    // Color value for LED off (black/no color)
    };

    String getCurrentColor() // Returns the current color as a string.
    {
        switch (currentColor)
        {
        case Colors::Red:
            return "Red";
        case Colors::Green:
            return "Green";
        case Colors::Blue:
            return "Blue";
        case Colors::Off:
            return "Off";
        default:
            return "Unknown";
        }
    }

    Colors currentColor = Colors::Off; // Current color of the LED

    LEDController(uint8_t pin, uint8_t count) : strip(Adafruit_NeoPixel(count, pin, NEO_GRB + NEO_KHZ800)) {}

    void begin()
    {
        strip.begin();
        strip.show(); // Ensure the LED is off initially
    }

    void setColor(Colors color) // Method to set the LED color
    {
        currentColor = color; // Update the current color
        if (color != Colors::Off)
        {
            strip.fill(static_cast<uint32_t>(color), 0, STRIP_COUNT);
            strip.show();
        }
    }

    // Method to handle blinking logic
    void handleBlink(Colors blinkColor, unsigned long currentMillis) // Updated to be non-blocking
    {
        if (currentColor == blinkColor) // Check if the current color matches the blinking color
        {
            if (currentMillis - lastBlinkTime >= blinkInterval) // Check if the blink interval has passed
            {
                lastBlinkTime = currentMillis; // Update the last blink time
                ledState = !ledState;          // Toggle the state
                strip.fill(ledState ? static_cast<uint32_t>(blinkColor) : static_cast<uint32_t>(Colors::Off), 0, STRIP_COUNT);
                strip.show();
            }
        }
        else
        {
            // If not blinking, ensure LED color matches the current state
            setColor(currentColor);
        }
    }
};

// Class to control the buzzer for auditory alerts.
class BuzzerController
{
private:
    int buzzerPin; // Pin number attached to the buzzer.

    // Nested structure for buzzer configuration
    struct BuzzerAlertConfig
    {
        int frequency; // Frequency for the beep
        int duration;  // Duration for the beep
    };

public:
    // Enum to define different alert types
    enum AlertType
    {
        ALERT_MUTE,   // Mute alert type
        ALERT_LOW,    // Low alert type
        ALERT_MEDIUM, // Medium alert type
        ALERT_HIGH,   // High alert type
        ALERT_ERROR   // Error alert type
    };
    // Map each alert type to its corresponding BuzzerAlertConfig
    std::map<AlertType, BuzzerAlertConfig> alertConfigs;

    // Constructor that initializes the buzzer pin
    BuzzerController(int pin) : buzzerPin(pin)
    {
        // Initialize the buzzer pin
        pinMode(buzzerPin, OUTPUT);

        // Default configurations for different alert types
        alertConfigs[ALERT_MUTE] = {0, 0};        // Mute alert type, 0 Hz frequency, 0 ms duration
        alertConfigs[ALERT_LOW] = {1000, 100};    // Low alert type, 1000 Hz frequency, 100 ms duration
        alertConfigs[ALERT_MEDIUM] = {2000, 200}; // Medium alert type, 2000 Hz frequency, 200 ms duration
        alertConfigs[ALERT_HIGH] = {3000, 300};   // High alert type, 3000 Hz frequency, 300 ms duration
        alertConfigs[ALERT_ERROR] = {250, 1000};  // Error alert type, 250 Hz frequency, 1000 ms duration
    }

    // Function to set configuration for a specific alert type
    void setConfig(AlertType type, int frequency, int duration)
    {
        alertConfigs[type] = {frequency, duration};
    }

    // Function to trigger a beep for a specific alert type
    void beep(AlertType type)
    {
        if (type == ALERT_MUTE || alertConfigs.find(type) == alertConfigs.end())
        {
            noTone(buzzerPin); // Stop any ongoing tone if mute is selected or type not found
        }
        else
        {
            BuzzerAlertConfig config = alertConfigs[type];
            tone(buzzerPin, config.frequency, config.duration);
        }
    }
};

// Class to encapsulate temperature sensing and LED indication logic
class TemperatureSensor
{
private:
    temperature_sensor_handle_t tsens; // Temperature sensor handle
    float lowThreshold;                // Custom low temperature threshold
    float highThreshold;               // Custom high temperature threshold

public:
    enum TemperatureState
    { // Enumeration to represent different temperature states
        Normal,
        Low,
        High,
        Error
    };

    TemperatureSensor(float lowThreshold, float highThreshold) // Constructor with custom temperature thresholds
        : lowThreshold(lowThreshold), highThreshold(highThreshold)
    {
        // Initialize the temperature sensor here or in a separate init method
        temperature_sensor_config_t tsens_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80); // Default configuration
        esp_err_t result = temperature_sensor_install(&tsens_config, &tsens);                  // Install the temperature sensor
        if (result == ESP_OK)
        {
            result = temperature_sensor_enable(tsens);
            if (result != ESP_OK)
            {
                Serial.println("Failed to enable temperature sensor");
            }
        }
        else
        {
            Serial.println("Failed to initialize temperature sensor");
        }
    }

    float readTemperature()
    {                                                                           // Method to read the temperature from the sensor
        float temperature = 0.0;                                                // Variable to store the temperature reading
        esp_err_t result = temperature_sensor_get_celsius(tsens, &temperature); // Get the temperature reading
        if (result != ESP_OK)
        {
            Serial.println("Failed to read temperature");
            return NAN;
        }
        return temperature;
    }

    TemperatureState getTemperatureState(float temperature)
    { // Method to determine the temperature state
        if (isnan(temperature))
        {
            return Error;
        }
        else if (temperature < lowThreshold)
        {
            return Low;
        }
        else if (temperature > highThreshold)
        {
            return High;
        }
        else
        {
            return Normal;
        }
    }
};

// Global objects for LED, buzzer, and temperature sensor control
LEDController ledController(LED_PIN, STRIP_COUNT);                 // LED controller object
BuzzerController buzzerController(BUZZER_PIN);                     // Buzzer controller object
TemperatureSensor tempSensor(lowTempThreshold, highTempThreshold); // Temperature sensor object

void setup()
{
    Serial.begin(115200);
    ledController.begin();
}

void loop()
{
    unsigned long currentMillis = millis(); // Get the current time

    if (currentMillis - lastReadTime >= readInterval)
    {                                 // If it's time for another reading
        lastReadTime = currentMillis; // Update the last reading timestamp

        float temperatureC = tempSensor.readTemperature();                                        // Read the temperature in Celsius
        float temperatureF = temperatureC * 9 / 5 + 32;                                           // Convert to Fahrenheit
        TemperatureSensor::TemperatureState state = tempSensor.getTemperatureState(temperatureC); // Get the temperature state

        // Print the temperature readings
        Serial.print("Temperature: ");
        Serial.print(temperatureC);
        Serial.print(" °C, ");
        Serial.print(temperatureF);
        Serial.print(" °F, ");

        switch (state)
        {
        case TemperatureSensor::Low:                             // Update the temperature state and LED based on the reading
            ledController.setColor(LEDController::Colors::Blue); // Set the LED color to blue
            break;
        case TemperatureSensor::High:
            ledController.setColor(LEDController::Colors::Red); // Will blink in handleBlink
            break;
        case TemperatureSensor::Normal:
            ledController.setColor(LEDController::Colors::Green); // Set the LED color to green
            break;
        case TemperatureSensor::Error:
            ledController.setColor(LEDController::Colors::Red); // Set the LED color to red
            break;
        }

        // Print the current color after setting it
        Serial.println("Current LED Color: " + ledController.getCurrentColor());
    }

    // Call handleBlink regardless of the temperature reading logic
    ledController.handleBlink(LEDController::Colors::Red, currentMillis);
}
