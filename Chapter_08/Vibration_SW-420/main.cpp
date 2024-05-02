// **********************************
// Project: Vibration Detection System
// Created by: Senior Software Developer
// Creation Date: 2024-05-02
// IDE: PlatformIO
// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This code is designed to detect vibration presence using a vibration sensor connected to an ESP32 C3.
// It utilizes an RGB LED and a Piezo Buzzer to provide visual and auditory alerts based on vibration detection.
// Hardware Connection:
// - VIBRATION D0 pin -> GPIO0
// - RGB LED Red -> GPIO2, Green -> GPIO3, Blue -> GPIO10
// - Piezo Buzzer -> GPIO11
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>

// **********************************
// Constants and Variables Declaration
// **********************************
constexpr int VIBRATION_PIN = 0; // Pin connected to the VIBRATION sensor

constexpr unsigned long SENSOR_READ_INTERVAL = 1000; // Read interval in milliseconds
unsigned long lastCheckTime = 0;                     // Last time the sensor was checked
constexpr int LED_RED_PIN = 2;                       // Red LED pin
constexpr int LED_GREEN_PIN = 3;                     // Green LED pin
constexpr int LED_BLUE_PIN = 10;                     // Blue LED pin

// * Buzzer Pins and PWM settings
constexpr int BUZZER_PIN = 11;          // Pin connected to the Piezo Buzzer
constexpr int BUZZER_CHANNEL = 0;       // LEDC channel for Piezo Buzzer
constexpr int BUZZER_FREQUENCY = 2000;  // Frequency for Buzzer PWM
constexpr int BUZZER_RESOLUTION = 10;   // Resolution for Buzzer PWM (10-bit = 0-1023)
constexpr int BUZZER_VOLUME_HALF = 512; // Half volume for the buzzer
constexpr int BUZZER_OFF = 0;           // Turn off the buzzer

bool isBuzzerOn = false; // Track whether the buzzer should be considered ON or OFF

// **********************************
// Funcion Declaration
// **********************************
bool readVibrationSensor();                         // Function to read the vibration sensor
void updateIndicatorStatus(bool VibrationDetected); // Function to control outputs based on sensor readings
void beepBuzzerAlert(bool activate);                // Function to activate buzzer
void printSystemStatus(bool VibrationDetected);     // Function to print system status

// **********************************
// Setup Function
// **********************************
void setup()
{
    Serial.begin(115200);           // Start serial communication
    pinMode(VIBRATION_PIN, INPUT);  // Set the sensor pin as input
    pinMode(LED_RED_PIN, OUTPUT);   // Set Red LED pins as output
    pinMode(LED_GREEN_PIN, OUTPUT); // Set Green LED pins as output
    pinMode(LED_BLUE_PIN, OUTPUT);  // Set Blue LED pins as output
    pinMode(BUZZER_PIN, OUTPUT);    // Set buzzer pin as output

    // Setup and Attach PWM channels to the Piezo Buzzer
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQUENCY, BUZZER_RESOLUTION); // Setup LEDC channel for Piezo Buzzer
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);                      // Attach Piezo Buzzer to PWM channel

    // Initial LED state setup based on initial sensor read
    bool initialVibrationDetected = readVibrationSensor(); // Read initial sensor value
    updateIndicatorStatus(initialVibrationDetected);       // Update LED state
    beepBuzzerAlert(initialVibrationDetected);             // Activate buzzer based on initial sensor value
}

// **********************************
// Main Loop
// **********************************
void loop()
{
    if (millis() - lastCheckTime >= SENSOR_READ_INTERVAL)
    {
        lastCheckTime = millis();
        bool VibrationDetected = readVibrationSensor();
        updateIndicatorStatus(VibrationDetected);
        beepBuzzerAlert(VibrationDetected);
        printSystemStatus(VibrationDetected); // Print system status for debugging and monitoring
    }
}

// **********************************
// Function Definitions
// **********************************
bool readVibrationSensor() // Function to read the VIBRATION sensor
{
    int sensorValue = digitalRead(VIBRATION_PIN); // Read the sensor value
    return (sensorValue == HIGH);                 // Return true if the sensor value is high
}

void updateIndicatorStatus(bool VibrationDetected) // Function to control outputs based on sensor readings
{
    static bool lastRedState = !VibrationDetected;  // Assuming red LED is used for this application
    static bool lastGreenState = VibrationDetected; // Assuming green LED is used for this application
    if (lastRedState != VibrationDetected)          // If the state has changed
    {
        digitalWrite(LED_RED_PIN, VibrationDetected ? HIGH : LOW); // Update the LED state
        lastRedState = VibrationDetected;                          // Update the last state
    }
    if (lastGreenState != !VibrationDetected) // If the state has changed
    {
        digitalWrite(LED_GREEN_PIN, !VibrationDetected ? HIGH : LOW); // Update the LED state
        lastGreenState = !VibrationDetected;                          // Update the last state
    }
    digitalWrite(LED_BLUE_PIN, HIGH); // Assuming blue LED is not used for this application

#ifdef DEBUG
    Serial.print("VIBRATION Detected: ");
    Serial.println(VibrationDetected ? "Yes" : "No");
#endif
}

void beepBuzzerAlert(bool activate) // Function to activate buzzer
{
    if (activate) // If vibration is detected
    {
        ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME_HALF); // Set to half volume
        isBuzzerOn = true;                             // Update the buzzer state
    }
    else
    {
        ledcWrite(BUZZER_CHANNEL, BUZZER_OFF); // Turn off the buzzer
        isBuzzerOn = false;                    // Update the buzzer state
    }
}

void printSystemStatus(bool VibrationDetected) // Function to print system status
{
    Serial.print("Vibration Detected: ");
    Serial.println(VibrationDetected ? "YES" : "NO");
    Serial.print("Red LED State: ");
    Serial.println(digitalRead(LED_RED_PIN) == HIGH ? "ON" : "OFF");
    Serial.print("Green LED State: ");
    Serial.println(digitalRead(LED_GREEN_PIN) == HIGH ? "ON" : "OFF");
    Serial.print("Buzzer State: ");
    Serial.println(isBuzzerOn ? "ON" : "OFF");
}
