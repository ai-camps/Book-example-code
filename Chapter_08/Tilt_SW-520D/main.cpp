// **********************************
// Project: Tilt Detection System
// Created by: Senior Software Developer
// Creation Date: 2024-05-02
// IDE: PlatformIO
// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This code is designed to detect tilt presence using a tilt sensor connected to an ESP32 C3.
// It utilizes an RGB LED and a Piezo Buzzer to provide visual and auditory alerts based on tilt detection.
// Hardware Connection:
// - Tilt D0 pin -> GPIO0
// - RGB LED Red -> GPIO2, Green -> GPIO3, Blue -> GPIO10
// - Piezo Buzzer -> GPIO11
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>

// **********************************
// Constants and Variables Declaration
// **********************************
constexpr int TILT_PIN = 0; // Pin connected to the tilt sensor

// * LED PINs and PWM settings
constexpr int LED_RED_PIN = 2;   // Red LED pin
constexpr int LED_GREEN_PIN = 3; // Green LED pin
constexpr int LED_BLUE_PIN = 10; // Blue LED pin

constexpr int PWM_LED_RED_CHANNEL = 1;   // PWM channel for Red LED
constexpr int PWM_LED_GREEN_CHANNEL = 2; // PWM channel for Green LED
constexpr int PWM_LED_BLUE_CHANNEL = 3;  // PWM channel for Blue LED

constexpr int PWM_LED_FREQUENCY = 5000; // Suitable frequency for LEDs
constexpr int PWM_LED_RESOLUTION = 8;   // 8-bit resolution (0-255)

// * Buzzer Pins and PWM settings
constexpr int BUZZER_PIN = 11;              // Pin connected to the Piezo Buzzer
constexpr int PWM_BUZZER_CHANNEL = 0;       // PWM channel for Piezo Buzzer
constexpr int PWM_BUZZER_FREQUENCY = 2000;  // Frequency for Buzzer PWM
constexpr int PWM_BUZZER_RESOLUTION = 10;   // Resolution for Buzzer PWM (10-bit = 0-1023)
constexpr int PWM_BUZZER_VOLUME_HALF = 512; // Half volume for the buzzer
constexpr int PWM_BUZZER_OFF = 0;           // Turn off the buzzer

constexpr unsigned long SENSOR_READ_INTERVAL = 1000; // Read interval in milliseconds
unsigned long lastCheckTime = 0;                     // Last time the sensor was checked

int currentRedState = 0; // 0 means off, values > 0 mean on (at various intensities)
int currentGreenState = 0;
int currentBlueState = 0;

bool isBuzzerOn = false; // Track whether the buzzer should be considered ON or OFF

// **********************************
// Funcion Declaration
// **********************************
bool readTiltSensor();                          // Function to read the tilt sensor
void updateIndicatorStatus(bool TiltDetected);  // Function to control outputs based on sensor readings
void beepBuzzerAlert(bool activate);            // Function to activate buzzer
void printSystemStatus(bool VibrationDetected); // Function to print system status

// **********************************
// Setup Function
// **********************************
void setup()
{
    Serial.begin(115200);
    pinMode(TILT_PIN, INPUT);

    // * LED PWM setup
    ledcSetup(PWM_LED_RED_CHANNEL, PWM_LED_FREQUENCY, PWM_LED_RESOLUTION);   // Setup PWM channel for Red LED
    ledcAttachPin(LED_RED_PIN, PWM_LED_RED_CHANNEL);                         // Attach Red LED to PWM channel
    ledcSetup(PWM_LED_GREEN_CHANNEL, PWM_LED_FREQUENCY, PWM_LED_RESOLUTION); // Setup PWM channel for Green LED
    ledcAttachPin(LED_GREEN_PIN, PWM_LED_GREEN_CHANNEL);                     // Attach Green LED to PWM channel
    ledcSetup(PWM_LED_BLUE_CHANNEL, PWM_LED_FREQUENCY, PWM_LED_RESOLUTION);  // Setup PWM channel for Blue
    ledcAttachPin(LED_BLUE_PIN, PWM_LED_BLUE_CHANNEL);                       // Attach Blue LED to PWM channel

    // * Buzzer PWM setup
    ledcSetup(PWM_BUZZER_CHANNEL, PWM_BUZZER_FREQUENCY, PWM_BUZZER_RESOLUTION); // Setup PWM channel for Buzzer
    ledcAttachPin(BUZZER_PIN, PWM_BUZZER_CHANNEL);                              // Attach Buzzer to PWM channel

    // * Initial LED state setup based on initial sensor read
    bool initialTiltDetected = readTiltSensor(); // Read the tilt sensor
    updateIndicatorStatus(initialTiltDetected);  // Update the LED status based on sensor reading
    beepBuzzerAlert(initialTiltDetected);        // Activate buzzer based on sensor reading
}

// **********************************
// Main Loop
// **********************************
void loop()
{
    if (millis() - lastCheckTime >= SENSOR_READ_INTERVAL)
    {
        lastCheckTime = millis();
        bool TiltDetected = readTiltSensor(); // Read the tilt sensor
        updateIndicatorStatus(TiltDetected);  // Update the LED status based on sensor reading
        beepBuzzerAlert(TiltDetected);        // Activate buzzer based on sensor reading
        printSystemStatus(TiltDetected);      // Print system status for debugging and monitoring
    }
}

// **********************************
// Function Definitions
// **********************************
bool readTiltSensor() // Function to read the TILT sensor
{
    int sensorValue = digitalRead(TILT_PIN); // Read the sensor value
    return (sensorValue == HIGH);            // Return true if the sensor value is high
}

void updateIndicatorStatus(bool TiltDetected) // Function to control outputs based on sensor readings
{
    if (TiltDetected) // If tilt is detected
    {
        ledcWrite(PWM_LED_RED_CHANNEL, 255); // Set to full volume
        currentRedState = 255;               // Update the red state
        ledcWrite(PWM_LED_GREEN_CHANNEL, 0); // Set to off
        currentGreenState = 0;               // Update the green state
        ledcWrite(PWM_LED_BLUE_CHANNEL, 0);  // Set to off
        currentBlueState = 0;                // Update the blue state
    }
    else
    {
        ledcWrite(PWM_LED_RED_CHANNEL, 0);     // Set to off
        currentRedState = 0;                   // Update the red state
        ledcWrite(PWM_LED_GREEN_CHANNEL, 255); // Set to full volume
        currentGreenState = 255;               // Update the green state
        ledcWrite(PWM_LED_BLUE_CHANNEL, 0);    // Set to off
        currentBlueState = 0;                  // Update the blue state
    }
}

void beepBuzzerAlert(bool activate) // Function to activate buzzer
{
    if (activate) // If tilt is detected
    {
        ledcWrite(PWM_BUZZER_CHANNEL, PWM_BUZZER_VOLUME_HALF); // Set to half volume
        isBuzzerOn = true;                                     // Update the buzzer state
    }
    else
    {
        ledcWrite(PWM_BUZZER_CHANNEL, PWM_BUZZER_OFF); // Turn off the buzzer
        isBuzzerOn = false;                            // Update the buzzer state
    }
}

void printSystemStatus(bool TiltDetected) // Function to print system status
{
    Serial.print("PIN Value - "); 
    Serial.println(digitalRead(TILT_PIN));
    Serial.print("Tilt Detected: ");
    Serial.println(TiltDetected ? "YES" : "NO");
    Serial.print("Red LED State: ");
    Serial.println(currentRedState > 0 ? "ON" : "OFF");
    Serial.print("Green LED State: ");
    Serial.println(currentGreenState > 0 ? "ON" : "OFF");
    Serial.print("Blue LED State: ");
    Serial.println(currentBlueState > 0 ? "ON" : "OFF");
    Serial.print("Buzzer State: ");
    Serial.println(isBuzzerOn ? "ON" : "OFF");
}
