// **********************************
// Created by: ESP32-C6 Coding Assistant
// Creation Date: 2024-02-23
// **********************************
// **********************************
// Code Explanation
// **********************************
// This code demonstrates how to interface with a BMP180 sensor to read air pressure values,
// utilize the ESP32-C6's built-in LED for visual alerts based on the air pressure levels, and
// connect a Piezo Buzzer for auditory alerts under specific conditions.
// **********************************
// Requirement Summary
// **********************************
// - Interface with BMP180 sensor via I2C.
// - Utilize ESP32-C6's built-in LED to indicate air pressure levels and errors.
// - Use a Piezo Buzzer for alerts in specific conditions.
// - Periodically read out air pressure values and print them locally.
// **********************************
// Hardware Connection
// **********************************
// BMP180 Sensor connected to ESP32-C6 via I2C (SDA: GPIO21, SCL: GPIO22).
// Built-in LED on GPIO9 (as per ESP32-C6's specifications).
// Piezo Buzzer connected to a designated GPIO pin (e.g., GPIO10).
// **********************************
// New Created Functions and Classes
// **********************************
// LEDController - Class to control the LED based on air pressure
// BuzzerController - Class to control the buzzer for auditory alerts.
// **********************************
// Security Considerations:
// **********************************
// Implements error handling for robust sensor communication.
// **********************************
// Testing and Validation Approach:
// **********************************
// The code should be tested with varying air pressure levels to ensure accurate feedback through the LED.
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>           // Includes the main Arduino library for core functionality.
#include <Adafruit_NeoPixel.h> // Includes the library for controlling NeoPixel LEDs.
#include <Adafruit_BMP085.h>   // Includes the library for interfacing with the BMP180 sensor.
#include <Wire.h>              // Includes the Wire library for I2C communication with the BMP180 sensor.
#include <SPI.h>               // Includes the SPI library, unnecessary for this specific application and can be removed.
#include <map>                 // Includes the map library for mapping alert types to their corresponding configurations.

// **********************************
// Constants Declaration
// **********************************
#define BMP180_SDA 21 // Defines the GPIO pin used for the BMP180 sensor's SDA line.
#define BMP180_SCL 22 // Defines the GPIO pin used for the BMP180 sensor's SCL line.

#define BUZZER_PIN 10 // Defines the pin number connected to the buzzer.

// **********************************
// Color Definitions
// **********************************

Adafruit_BMP085 bmp; // Creates an instance of the BMP180 sensor object.

// **********************************
// Air Pressure Shreshold Configuration
// **********************************
struct AirPressureConfig
{
  float lowPressureThreshold;    // Threshold for low air pressure
  float mediumPressureThreshold; // Threshold for medium air pressure
  float highPressureThreshold;   // Threshold for high air pressure
};

// Define a global configuration instance with initial threshold values
AirPressureConfig pressureConfig = {450.0, 650.0, 1013.0};

// **********************************
// Class Definitions
// **********************************
// Class to control the LED based on air pressure
class LEDController
{
private:

#define LED_PIN 8                          // Defines the pin number connected to the NeoPixel LED.
#define STRIP_COUNT 1                      // Defines the number of the strip 
  int pixelIndex = 0;                      // Index of the pixel to control.
  Adafruit_NeoPixel strip;                 // NeoPixel object to control the LED strip.
  unsigned long lastBlinkTime = 0;         // Tracks the time since the last LED blink.
  const unsigned long blinkInterval = 500; // Interval between blinks in milliseconds.
  bool ledState = false;                   // Current state of the LED (on or off).

public:

  enum class Colors : uint32_t
  {
    Red = 0xFF0000,   // Color value for red
    Green = 0x00FF00, // Color value for green
    Blue = 0x0000FF,  // Color value for blue
    Off = 0x000000    // Color value for LED off
  };

  // Constructor that initializes the NeoPixel strip object with the specified pin number and number of LEDs.
  LEDController(uint8_t pin, uint8_t count) : strip(Adafruit_NeoPixel(count, pin, NEO_GRB + NEO_KHZ800)) {}

  void begin()
  {
    strip.begin(); // Initializes the NeoPixel strip.
    strip.show();  // Clears the strip at startup.
  }

  void setColor(Colors color)
  {
    uint32_t colorValue = static_cast<uint32_t>(color);       // Optimization: Avoids repeated static_cast.
    strip.fill(static_cast<uint32_t>(color), pixelIndex, STRIP_COUNT); // Sets the entire strip to a specified color.
    strip.show();                                             // Updates the strip to show the new color.
  }

  void blinkLED(Colors color, unsigned long currentMillis)
  {
    if (currentMillis - lastBlinkTime >= blinkInterval)
    {                       // Checks if it's time to toggle the LED state.
      ledState = !ledState; // Toggles the LED state.
      // Sets the strip color based on the current state or turns it off.
      strip.fill(ledState ? static_cast<uint32_t>(Colors::Red) : static_cast<uint32_t>(Colors::Off), pixelIndex, STRIP_COUNT);
      strip.show();                  // Updates the strip to reflect the change.
      lastBlinkTime = currentMillis; // Resets the timer for the next blink.
    }
  }
};

// **********************************
// Class to control the buzzer
// **********************************
// Class to control the buzzer for auditory alerts.
class BuzzerController
{
private:
  int buzzerPin;      // Pin number attached to the buzzer.

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

LEDController ledController(LED_PIN, STRIP_COUNT); // Instantiates the LEDController with the specified pin and strip count.
BuzzerController buzzerController(BUZZER_PIN);     // Instantiates the BuzzerController with the specified pin.

void setup()
{
  Serial.begin(115200);               // Begins serial communication at 115200 baud.
  Wire.begin(BMP180_SDA, BMP180_SCL); // Initializes I2C communication with the BMP180 sensor.
  if (!bmp.begin())
  {
    Serial.println("BMP180 init failed! Attempting to continue without sensor."); // Prints an error message if the sensor is not found.
  }
  ledController.begin(); // Initializes the LED strip.
}

void loop()
{
  static unsigned long lastReadTime = 0;    // Declare a static variable to keep track of the last time the sensor was read. Static ensures the value persists across function calls.
  const unsigned long READ_INTERVAL = 5000; // Define the interval between sensor readings in milliseconds (5000ms = 5 seconds).
  unsigned long currentMillis = millis();   // Get the current time in milliseconds since the program started.

  // Check if the time since the last sensor reading is greater than or equal to the defined interval.
  if (currentMillis - lastReadTime >= READ_INTERVAL)
  {
    lastReadTime = currentMillis; // Update lastReadTime with the current time for the next interval comparison.

    // Attempt to initialize the BMP180 sensor. If initialization fails, execute the following block.
    if (!bmp.begin())
    {
      ledController.setColor(LEDController::Colors::Red);   // Set the LED color to red to indicate a sensor error.
      buzzerController.beep(BuzzerController::ALERT_ERROR); // Trigger an error beep using the buzzer.
      return;                                               // Exit the loop function early to skip further processing.
    }

    float pressure = bmp.readPressure() / 100.0; // Read the air pressure from the sensor and convert it to hectopascals (hPa).

    // Check if the pressure is below the low threshold.
    if (pressure < pressureConfig.lowPressureThreshold)
    {
      ledController.blinkLED(LEDController::Colors::Red, currentMillis); // Make the LED blink red for low pressure.
      buzzerController.beep(BuzzerController::ALERT_HIGH);               // Trigger a low pressure beep.
    }
    // Check if the pressure is within the medium threshold range.
    else if (pressure > pressureConfig.lowPressureThreshold && pressure <= pressureConfig.mediumPressureThreshold)
    {
      ledController.setColor(LEDController::Colors::Green);   // Set the LED to a steady blue color for medium pressure.
      buzzerController.beep(BuzzerController::ALERT_LOW); // Trigger a medium pressure beep.
    }
    // Check if the pressure is within the high threshold range but not exceeding the high limit.
    else if (pressure > pressureConfig.mediumPressureThreshold && pressure <= pressureConfig.highPressureThreshold)
    {
      ledController.setColor(LEDController::Colors::Blue); // Set the LED to a steady green color for normal to high pressure. No beep is triggered.
      buzzerController.beep(BuzzerController::ALERT_MEDIUM);  // mute beep.
    }
  }
}
