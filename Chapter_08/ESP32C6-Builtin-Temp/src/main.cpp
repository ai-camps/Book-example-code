// **********************************
// Code Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-25
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
#include <ESP32Info.h>                  // Library for accessing ESP32-C6 hardware information
#include "builtin_temperature_sensor.h" // Library for accessing the ESP32-C6's built-in temperature sensor
#include <Adafruit_NeoPixel.h>          // Library for controlling RGB LEDs, the built-in one on ESP32-C6
#include <map>                          // Includes the map library for mapping alert types to their corresponding configurations.

// Define constants for LED and buzzer control
#define LED_PIN 8     // Pin number for the NeoPixel LED
#define STRIP_COUNT 1 // Number of LEDs in the strip (1 for built-in)
#define BUZZER_PIN 4  // Pin number for the buzzer

// Global variables for temperature reading and LED blinking control
int pixelIndex = 0;                // Index for the controlled LED (only one in this case)
unsigned long readInterval = 5000; // Interval between temperature readings (milliseconds)
unsigned long lastReadTime = 0;    // Timestamp of the last temperature reading
unsigned long blinkInterval = 500; // Blink interval for the LED (milliseconds)
unsigned long lastBlinkTime = 0;   // Timestamp of the last LED blink
bool ledBlinkState = false;        // Current state of LED blinking (on or off)

// Temperature thresholds for indicating different states
float lowTempThreshold = 15.0;  // Threshold for low temperature
float highTempThreshold = 25.0; // Threshold for high temperature

class BuzzerController {
private:
    int buzzerPin; // Pin attached to the buzzer

    // Structure for buzzer configuration (frequency and duration)
    struct BuzzerAlertConfig {
        int frequency;
        int duration;
    };

public:
    enum AlertType {
        ALERT_MUTE,   // No sound
        ALERT_LOW,    // Low temperature alert
        ALERT_MEDIUM, // Medium temperature alert
        ALERT_HIGH,   // High temperature alert
        ALERT_ERROR   // Sensor error alert
    };

    std::map<AlertType, BuzzerAlertConfig> alertConfigs; // Maps alert types to configurations

    // Constructor initializes the buzzer pin and default configurations
    BuzzerController(int pin) : buzzerPin(pin) {
        pinMode(buzzerPin, OUTPUT); // Set the buzzer pin as an output

        // Initialize default configurations for each alert type
        alertConfigs[ALERT_MUTE] = {0, 0};
        alertConfigs[ALERT_LOW] = {1000, 100};
        alertConfigs[ALERT_MEDIUM] = {2000, 200};
        alertConfigs[ALERT_HIGH] = {3000, 300};
        alertConfigs[ALERT_ERROR] = {250, 1000};
    }

    // Plays a beep sound based on the specified alert type
    void beep(AlertType type) {
        if (type == ALERT_MUTE || alertConfigs.find(type) == alertConfigs.end()) {
            noTone(buzzerPin); // Stop any ongoing tone
        } else {
            BuzzerAlertConfig config = alertConfigs[type];
            tone(buzzerPin, config.frequency, config.duration);
        }
    }

    // Stops any ongoing tone, effectively muting the buzzer
    void mute() {
        noTone(buzzerPin);
    }
};

class LEDController {
private:
    Adafruit_NeoPixel strip;         // NeoPixel strip object
    bool ledState = false;           // Current LED state (on/off) for blinking
    unsigned long lastBlinkTime = 0; // Timestamp of the last blink action

public:
    enum class Colors {
        Red = 0xFF0000,
        Green = 0x00FF00,
        Blue = 0x0000FF,
        Off = 0x000000 // Represents the LED turned off
    };
    String getCurrentLEDColor() // Returns the current color as a string.
    {
        switch (currentLEDColor)
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
    // Current color of the LED, initialized to off
    Colors currentLEDColor = Colors::Off;

    // Constructor initializes the NeoPixel strip
    LEDController(uint8_t pin, uint8_t count) : strip(Adafruit_NeoPixel(count, pin, NEO_GRB + NEO_KHZ800)) {}

    // Initializes the LED strip
    void begin() {
        strip.begin();
        strip.show(); // Turn off the LED initially
    }

    // Sets the LED color
    void setColor(Colors color) {
        currentLEDColor = color; // Update the current LED color
        strip.fill(static_cast<uint32_t>(color), 0, STRIP_COUNT); // Set the color on the strip
        strip.show(); // Apply the color change
    }

    // Handles the blinking logic for the LED based on the specified color
    void handleBlink(Colors blinkColor, unsigned long currentMillis, BuzzerController &buzzer) {
        // Only proceed if blinking is necessary (currentColor matches blinkColor or ledState is true)
        if (currentLEDColor != blinkColor && !ledState) return;

        if (currentMillis - lastBlinkTime >= blinkInterval) { // Check if it's time to toggle the blink state
            lastBlinkTime = currentMillis; // Update the last blink time
            ledState = !ledState; // Toggle the LED state

            // Set the LED color based on the current blink state
            strip.show();
            strip.fill(ledState ? static_cast<uint32_t>(blinkColor) : static_cast<uint32_t>(Colors::Off), 0, STRIP_COUNT);
            strip.show();
            
            // Control the buzzer based on the LED state
            if (ledState) {
                buzzer.beep(BuzzerController::ALERT_HIGH); // Beep when the LED is on
            } else {
                buzzer.mute(); // Mute the buzzer when the LED is off
            }
        }
    }
};

class TemperatureSensor {
private:
    temperature_sensor_handle_t tsens; // Handle for the temperature sensor
    float lowThreshold;                // Low temperature threshold
    float highThreshold;               // High temperature threshold

public:
    enum TemperatureState {
        Normal,
        Low,
        High,
        Error
    };

    // Constructor with custom temperature thresholds
    TemperatureSensor(float lowThreshold, float highThreshold)
        : lowThreshold(lowThreshold), highThreshold(highThreshold) {
        temperature_sensor_config_t tsens_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
        esp_err_t result = temperature_sensor_install(&tsens_config, &tsens);

        if (result == ESP_OK) {
            result = temperature_sensor_enable(tsens);
            if (result != ESP_OK) {
                Serial.println("Failed to enable temperature sensor. Check hardware connections.");
            }
        } else {
            Serial.println("Failed to initialize temperature sensor. Check hardware connections.");
        }
    }

    // Reads the temperature from the sensor
    float readTemperature() {
        static unsigned long lastErrorTime = 0; // Timestamp of the last sensor error
        if (millis() - lastErrorTime < 6000) { // Wait for 60 seconds after an error
            Serial.println("Waiting for sensor cooldown...");
            return NAN;
        }

        float temperature = 0.0;
        esp_err_t result = temperature_sensor_get_celsius(tsens, &temperature);

        if (result != ESP_OK) {
            lastErrorTime = millis(); // Update the last error time
            Serial.println("Failed to read temperature. Entering cooldown.");
            return NAN;
        }
        return temperature;
    }

    // Determines the state of the temperature (Normal, Low, High, Error)
    TemperatureState getTemperatureState(float temperature) {
        if (isnan(temperature)) {
            return Error; // Temperature read failure
        } else if (temperature < lowThreshold) {
            return Low; // Low temperature
        } else if (temperature > highThreshold) {
            return High; // High temperature
        } else {
            return Normal; // Normal temperature
        }
    }
};

// Global objects for controlling the LED, buzzer, and temperature sensor
LEDController ledController(LED_PIN, STRIP_COUNT);
BuzzerController buzzerController(BUZZER_PIN);
TemperatureSensor tempSensor(lowTempThreshold, highTempThreshold);

void setup() {
    Serial.begin(115200); // Initialize serial communication
    ledController.begin(); // Initialize the LED controller
    ESP32Info::initializeSerial(); // Initialize serial connection for ESP32 information
    ESP32Info::printChipInfo(); // Print information about the ESP32-C6 chip
}

void loop() {
    unsigned long currentMillis = millis(); // Get the current system time

    // Periodically read the temperature and update the LED and buzzer accordingly
    if (currentMillis - lastReadTime >= readInterval) {
        lastReadTime = currentMillis; // Update the last read time

        float temperatureC = tempSensor.readTemperature(); // Read temperature in Celsius
        float temperatureF = temperatureC * 9 / 5 + 32; // Convert to Fahrenheit
        TemperatureSensor::TemperatureState state = tempSensor.getTemperatureState(temperatureC); // Determine temperature state

        // Print the temperature readings
        Serial.print("Temperature: ");
        Serial.print(temperatureC);
        Serial.print(" °C, ");
        Serial.print(temperatureF);
        Serial.print(" °F, ");

        // Update LED and buzzer based on temperature state
        switch (state) {
            case TemperatureSensor::Low:
                ledController.setColor(LEDController::Colors::Blue); // Set LED to blue
                buzzerController.beep(BuzzerController::ALERT_LOW); // Low temperature alert
                break;
            /* case TemperatureSensor::High:
                ledController.setColor(LEDController::Colors::Red); // Set LED to blink red
                buzzerController.beep(BuzzerController::ALERT_HIGH); // High temperature alert
                break; */
            case TemperatureSensor::Normal:
                ledController.setColor(LEDController::Colors::Green); // Set LED to green
                buzzerController.beep(BuzzerController::ALERT_MUTE); // Medium temperature alert (no sound)    
                break;
            case TemperatureSensor::Error:
                ledController.setColor(LEDController::Colors::Red); // Set LED to steady red
                buzzerController.beep(BuzzerController::ALERT_ERROR); // Sensor error alert
                break;
        }

        // Print the current color after setting it
        Serial.println("Current LED Color: " + ledController.getCurrentLEDColor());
    }

    // Handle LED blinking independently of temperature reading
    ledController.handleBlink(LEDController::Colors::Red, currentMillis, buzzerController);
}
