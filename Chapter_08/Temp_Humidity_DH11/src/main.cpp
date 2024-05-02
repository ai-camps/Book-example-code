// **********************************
// Created by: ESP32 Coding Assistant
// Creation Date: 2024-05-01
// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This code is designed to monitor temperature and humidity using a DHT11 sensor.
// It provides visual and audible alerts using an RGB LED and a Piezo Buzzer based on sensor readings.
// Requirement Summary:
// - Periodically read from DHT11 sensor every 3 seconds.
// - Use LEDs and Piezo Buzzer for indication based on predefined thresholds.
// Hardware Connection:
// - DHT11 data pin -> GPIO0
// - RGB LED Red -> GPIO2, Green -> GPIO3, Blue -> GPIO10
// - Piezo Buzzer -> GPIO11
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>
#include "DHT.h"

// **********************************
// Constants and Variables Declaration
// **********************************
constexpr int DHT_PIN = 0;                           // Pin connected to the DHT11 data pin
constexpr int DHT_TYPE = DHT11;                      // Specify DHT11 type
constexpr float TEMP_NORMAL_LOW = 15.0;              // Lower boundary for normal temperature
constexpr float TEMP_NORMAL_HIGH = 25.0;             // Upper boundary for normal temperature
constexpr float HUM_NORMAL_LOW = 10.0;               // Lower boundary for normal humidity
constexpr float HUM_NORMAL_HIGH = 80.0;              // Upper boundary for normal humidity
constexpr unsigned long SENSOR_READ_INTERVAL = 3000; // Read interval in milliseconds
unsigned long lastCheckTime = 0;                     // Last time the sensor was checked
constexpr int BUZZER_PIN = 11;                       // Piezo Buzzer pin
constexpr int BUZZER_FREQUENCY = 2000;               // Frequency for buzzer beep
constexpr int BUZZER_DURATION = 100;                 // Duration of buzzer beep in milliseconds
constexpr int LED_RED_PIN = 2;                       // Red LED pin
constexpr int LED_GREEN_PIN = 3;                     // Green LED pin
constexpr int LED_BLUE_PIN = 10;                     // Blue LED pin
DHT dht(DHT_PIN, DHT_TYPE);                          // Initialize DHT sensor

// **********************************
// Funcion Declaration
// **********************************
void readDHTSensor(float &humidity, float &temperatureC, float &temperatureF); // Function to read DHT sensor
void updateIndicatorStatus(float humidity, float temperatureC);                // Function to control outputs based on sensor readings
void beepBuzzerAlert();                                                        // Function to activate buzzer

// **********************************
// Setup Function
// **********************************
void setup()
{
    Serial.begin(115200);           // Start serial communication
    dht.begin();                    // Initialize the DHT sensor
    pinMode(LED_RED_PIN, OUTPUT);   // Set Red LED pins as output
    pinMode(LED_GREEN_PIN, OUTPUT); // Set Green LED pins as output
    pinMode(LED_BLUE_PIN, OUTPUT);  // Set Blue LED pins as output
    pinMode(BUZZER_PIN, OUTPUT);    // Set buzzer pin as output
}

// **********************************
// Main Loop
// **********************************
void loop()
{
    if (millis() - lastCheckTime >= SENSOR_READ_INTERVAL)
    {
        lastCheckTime = millis();                                     // Update last check time
        float humidity = 0.0, temperatureC = 0.0, temperatureF = 0.0; // Initialize variables
        readDHTSensor(humidity, temperatureC, temperatureF);          // Read sensor and update humidity and temperature

        if (!isnan(humidity) && !isnan(temperatureC))
        {                                                  // Check for valid readings
            updateIndicatorStatus(humidity, temperatureC); // Update LED indicators based on sensor readings
        }
        else
        {
            Serial.println("Valid readings not obtained. Skipping update."); // Inform about skip
        }
    }
}

// **********************************
// Function Definitions
// **********************************

void readDHTSensor(float &humidity, float &temperatureC, float &temperatureF)
{
    int retryCount = 0;
    while (retryCount < 3)
    { // Attempt to read sensor values up to 3 times
        humidity = dht.readHumidity();
        temperatureC = dht.readTemperature();
        temperatureF = dht.readTemperature(true);

        if (!isnan(humidity) && !isnan(temperatureC))
        {
            break; // Break the loop if valid readings are obtained
        }

        retryCount++;
        delay(100); // Delay before retrying
    }

    if (retryCount == 3)
    { // Check if all retries failed
        Serial.println("Failed to read from DHT sensor after retries!");
        return;
    }

    // Print the sensor values to serial for monitoring
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print("C / ");
    Serial.print(temperatureF);
    Serial.print("F, Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
}

void updateIndicatorStatus(float humidity, float temperatureC)
{
    bool isAlertNeeded = false;

    if (temperatureC >= TEMP_NORMAL_LOW && temperatureC <= TEMP_NORMAL_HIGH &&
        humidity >= HUM_NORMAL_LOW && humidity <= HUM_NORMAL_HIGH)
    {
        digitalWrite(LED_GREEN_PIN, HIGH); // Turn on green LED
        digitalWrite(LED_RED_PIN, LOW);    // Turn off red LED
        digitalWrite(LED_BLUE_PIN, LOW);   // Turn off blue LED
    }
    else if (temperatureC > TEMP_NORMAL_HIGH || humidity > HUM_NORMAL_HIGH)
    {
        digitalWrite(LED_RED_PIN, HIGH);  // Turn on red LED
        digitalWrite(LED_GREEN_PIN, LOW); // Turn off green LED
        digitalWrite(LED_BLUE_PIN, LOW);  // Turn off blue LED
        isAlertNeeded = true;
    }
    else if (temperatureC < TEMP_NORMAL_LOW || humidity < HUM_NORMAL_LOW)
    {
        digitalWrite(LED_BLUE_PIN, HIGH); // Turn on blue LED
        digitalWrite(LED_GREEN_PIN, LOW); // Turn off green LED
        digitalWrite(LED_RED_PIN, LOW);   // Turn off red LED
        isAlertNeeded = true;
    }

    if (isAlertNeeded) {
        beepBuzzerAlert();
    } else {
        noTone(BUZZER_PIN); // Ensure buzzer is off otherwise
    }
}

void beepBuzzerAlert()
{
    tone(BUZZER_PIN, BUZZER_FREQUENCY, BUZZER_DURATION); // Buzzer beeps
}
