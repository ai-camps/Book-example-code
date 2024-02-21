// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This code interfaces the ESP32-C6 with a DHT11 temperature and humidity sensor. It uses the built-in
// RGB LED to provide visual feedback based on the temperature and humidity readings.

// Requirement:
// 1. Read temperature and humidity from DHT11 sensor every 3 seconds.
// 2. Display temperature in Celsius and Fahrenheit, and humidity in percentage.
// 3. Use built-in RGB LED for visual indication of temperature and humidity ranges.
// 4. Handle sensor read errors with visual feedback and console messages.

// Hardware Connection:
// DHT11 Data Pin -> GPIO2 (Refer to ESP32-C6-Pinout for available pins)
// Built-in RGB LED controlled via GPIO8

// New Created Functions:
// - initDHTSensor(): Initializes the DHT11 sensor.
// - readDHTSensor(): Reads and prints the temperature and humidity data.
// - setLEDColor(): Sets the built-in RGB LED color.

// Security Considerations:
// - Ensure stable power supply to prevent sensor malfunctions.
// - Validate sensor readings to handle unexpected spikes or drops.

// Testing and Validation Approach:
// - Test in various environmental conditions to ensure accurate readings.
// - Verify LED color changes according to specified temperature and humidity ranges.

// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>           // Includes the main Arduino library for general IO functions.
#include "ESP32Info.h"         // Import the ESP32Info library to get the ESP32-C6 information
#include <Adafruit_NeoPixel.h> // Incorporates the Adafruit NeoPixel library for controlling the WS2812B RGB LED.
#include "DHT.h"               // Includes the DHT sensor library for interfacing with the DHT11 sensor.

// **********************************
// Constants Declaration
// **********************************
#define dhtPin 4      // Pin connected to the DHT11 data line
#define dhtType DHT11 // Specify using a DHT11 sensor
#define ledStrip 1    // LED strip count, as ESP32-C6 has one built-in RGB LED
#define ledGpio 8     // LED GPIO PIN, which is GPIO8 for ESP32-C6
int pixelIndex = 0;   // Index of the LED you want to control

struct Colors
{
    static const uint32_t redColor = 0xFF0000;   // Red color
    static const uint32_t greenColor = 0x00FF00; // Green color
    static const uint32_t blueColor = 0x0000FF;  // Blue color
    static const uint32_t offColor = 0x000000;   // Off color
};

// **********************************
// Class Declaration
// **********************************
// Class Name: SensorLEDController
// Purpose: This class interfaces the ESP32-C6 with a DHT11 temperature and humidity sensor, and a WS2812B RGB LED.
// It reads temperature and humidity data every 3 seconds, and uses the RGB LED to provide visual feedback on the environmental conditions or sensor errors.
// It also provides methods for setting the RGB LED color, and for handling sensor read errors.
// The class is designed to be a simple and reusable component that can be integrated into other projects.
// The class is implemented using the Adafruit_NeoPixel library for controlling the WS2812B RGB LED, and the DHT library for interfacing with the DHT11 sensor.
// The class provides a simple interface for reading and handling sensor readings, and provides methods for setting the RGB LED color and handling sensor read errors.

class SensorLEDController
{
private:
    DHT dht;                                 // DHT sensor object
    Adafruit_NeoPixel strip;                 // Adafruit NeoPixel strip object
    unsigned long lastReadTime = 0;          // Last time the sensor was read
    unsigned long lastBlinkTime = 0;         // Last time the LED was blinked
    bool highConditionFlag = false;          // Flag to indicate high temperature or humidity condition
    bool ledState = false;                   // State of the LED
    const float lowTempThreshold = 10.0;     // Low temperature threshold
    const float highTempThreshold = 25.0;    // High temperature threshold
    const float lowHumThreshold = 10.0;      // Low humidity threshold
    const float highHumThreshold = 80.0;     // High humidity threshold
    const unsigned long readInterval = 3000; // Time interval for reading the temperature sensor in milliseconds
    const unsigned long blinkInterval = 100; // Time interval for red blinking in milliseconds

public:
    SensorLEDController(uint8_t sensorPin, uint8_t sensorType, uint8_t countOfLEDs, uint8_t gpioForLED)
        : dht(sensorPin, sensorType), strip(countOfLEDs, gpioForLED, NEO_GRB + NEO_KHZ800) {} // Constructor to initialize the DHT sensor and the Adafruit NeoPixel strip

    void begin()
    {
        Serial.begin(115200);
        strip.begin(); // Initialize the Adafruit NeoPixel strip
        strip.show();  // Turn off all LEDs at startup
        dht.begin();   // Initialize the DHT sensor
    }

    void update()
    {                                           // Update method to handle sensor readings and LED control
        unsigned long currentMillis = millis(); // Get the current time in milliseconds
        handleSensorReadings(currentMillis);    // Handle sensor readings
        handleLEDControl(currentMillis);        // Handle LED control
    }

private:
    void handleSensorReadings(unsigned long currentMillis)
    { // Method to handle sensor readings
        if (currentMillis - lastReadTime >= readInterval)
        {                                 // Check if the time interval for reading the sensor has elapsed
            lastReadTime = currentMillis; // Update the last read time
            readAndHandleSensor();        // Read and handle the sensor data
        }
    }

    void readAndHandleSensor()
    {                                        // Method to read and handle the sensor data
        float humidity = dht.readHumidity(); // Read the humidity from the DHT sensor
        float tempC = dht.readTemperature(); // Read the temperature from the DHT sensor
        if (isnan(humidity) || isnan(tempC))
        { // Check if the readings are valid
            Serial.println("Failed to read from DHT sensor!");
            setLEDColor(Colors::redColor, "Error"); // Set the LED color to red if the readings are invalid
            strip.show();                           // Update the LED strip
            highConditionFlag = false;              // Reset the high condition flag to false
        }
        else
        {
            Serial.printf("Humidity: %.1f%%, Temperature: %.1fC (%.1fF)\n", humidity, tempC, convertToFahrenheit(tempC)); // Print the humidity and temperature readings to the serial monitor
            evaluateCondition(tempC, humidity);                                                                           // Evaluate the temperature and humidity condition
        }
    }

    void evaluateCondition(float tempC, float humidity)
    { // Method to evaluate the temperature and humidity condition
        if (tempC < lowTempThreshold || humidity < lowHumThreshold)
        {                                                                   // Check if the temperature or humidity is below the low threshold
            setLEDColor(Colors::blueColor, "Blue for Cold/Dry Condition "); // Set the LED color to blue for cold/dry condition
            highConditionFlag = false;                                      // Reset the high condition flag to false
        }
        else if (tempC > highTempThreshold || humidity > highHumThreshold)
        {                             // Check if the temperature or humidity is above the high threshold
            highConditionFlag = true; // Set the high condition flag to true
        }
        else
        {
            setLEDColor(Colors::greenColor, "Green for Normal Condition"); // Set the LED color to green for normal condition
            highConditionFlag = false;                                     // Reset the high condition flag to false
        }
    }

    void handleLEDControl(unsigned long currentMillis)
    { // Method to handle LED control
        if (highConditionFlag)
        { // Check if the high condition flag is set
            if (currentMillis - lastBlinkTime > blinkInterval)
            {                                                                                                                               // Check if the time interval for blinking the LED has elapsed
                lastBlinkTime = currentMillis;                                                                                              // Update the last blink time
                ledState = !ledState;                                                                                                       // Toggle the LED state
                setLEDColor(ledState ? Colors::redColor : Colors::offColor, ledState ? "Blinking Red for Heat/Humidity Condition" : "Off"); // Set the LED color to blinking red if the high condition flag is set
            }
        }
        else if (ledState)
        {                                         // Check if the LED state is set
            ledState = false;                     // Reset the LED state to false
            setLEDColor(Colors::offColor, "Off"); // Set the LED color to off
        }
    }

    void setLEDColor(uint32_t color, const char *colorName)
    {                                           // Method to set the LED color
        strip.setPixelColor(pixelIndex, color); // Set the color of the LED strip
        strip.show();                           // Update the LED strip
        Serial.print("LED set to ");            // Print the LED color to the serial monitor
        Serial.println(colorName);              // Print the LED color name to the serial monitor
    }

    float convertToFahrenheit(float celsius)
    {                                    // Method to convert temperature from Celsius to Fahrenheit
        return celsius * 9.0 / 5.0 + 32; // Convert the temperature from Celsius to Fahrenheit
    }
};

SensorLEDController controller(dhtPin, dhtType, ledStrip, ledGpio); // Create an instance of the SensorLEDController class

void setup()
{
    controller.begin(); // Initialize the SensorLEDController
}

void loop()
{
    controller.update(); // Update the SensorLEDController
}