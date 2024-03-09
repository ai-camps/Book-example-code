// **********************************
// Created by: ESP32 Coding Assistant
// Creation Date: 2024-03-08
// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This code is designed to periodically read temperature and humidity data from a DHT11 sensor
// and provide visual and sound indications based on the data's normality or abnormality,
// and handle sensor reading errors.
// Requirement Summary:
// - Periodically read from DHT11 sensor connected to IO2.
// - Indicate conditions via LEDs and Piezo Buzzer connected to IO11, IO12, and IO13.
// Hardware Connection:
// - DHT11 data pin -> IO2
// - Piezo Buzzer -> IO11
// - RGB LED Red -> IO8, Green -> IO9, Blue -> IO4
// - LED D4 -> IO12, LED D5 -> IO13
// New Created Function/Class:
// - checkSensorReadings()
// - indicateNormalCondition()
// - indicateAbnormalCondition()
// - indicateSensorError()
// - ledBlinking()
// - updateLEDs()
// Security Considerations:
// - Ensure stable power supply to prevent erroneous readings.
// - Avoid exposing the sensor to extreme conditions beyond its operating range.
// Testing and Validation Approach:
// - Test under known temperature and humidity conditions to validate sensor readings.
// - Simulate error conditions by disconnecting the sensor.
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h> // Include the Arduino base library
#include "DHT.h"     // Include the library for the DHT sensor

// **********************************
// Constants Declaration
// **********************************

// DHT11 Sensor Pins and Settings
constexpr int DHTPIN = 2;      // Pin connected to the DHT11 data pin
constexpr int DHTTYPE = DHT11; // Specify DHT11 type

// RGB LED Pins
constexpr int LED_RED = 8;   // Pin for the red component of RGB LED
constexpr int LED_GREEN = 9; // Pin for the green component of RGB LED
constexpr int LED_BLUE = 4;  // Pin for the blue component of RGB LED

// LED D4 and D5 Pins and PWM settings
constexpr int LED_D4 = 12;              // Pin to LED D4, will be used for future use case.
constexpr int LED_ERROR_INDICATOR = 13; // Pin to LED (D5) for sensor reading error indication
constexpr int LED_D4_CHANNEL = 0;       // LEDC channel for range indicator LED (D4)
constexpr int LED_ERROR_CHANNEL = 1;    // LEDC channel for error indicator LED (D5)
constexpr int LED_FREQ = 5000;          // Frequency for LED PWM
constexpr int LED_RESOLUTION = 8;       // Resolution for LED PWM (8-bit = 0-255)
constexpr int LED_ON = 255;             // LED ON state
constexpr int LED_OFF = 0;              // LED OFF state

// Buzzer Pins and PWM settings
constexpr int BUZZER_PIN = 11;          // Pin connected to the Piezo Buzzer
constexpr int BUZZER_CHANNEL = 2;       // LEDC channel for Piezo Buzzer
constexpr int BUZZER_FREQ = 2000;       // Frequency for Buzzer PWM
constexpr int BUZZER_RESOLUTION = 10;   // Resolution for Buzzer PWM (10-bit = 0-1023)
constexpr int BUZZER_VOLUME_HALF = 512; // Half volume for the buzzer
constexpr int BUZZER_OFF = 0;           // Turn off the buzzer

// Normal operating ranges for temperature and humidity
constexpr float TEMP_MIN = 10.0; // Minimum normal temperature
constexpr float TEMP_MAX = 25.0; // Maximum normal temperature
constexpr float HUM_MIN = 10.0;  // Minimum normal humidity
constexpr float HUM_MAX = 80.0;  // Maximum normal humidity

// Sensor reading interval
constexpr unsigned long sensorReadInterval = 3000; // Interval between sensor readings
unsigned long lastCheckTime = 0;                   // Last sensor check time

// Blinking control variables
bool shouldBlink = false;           // Flag for LED blinking state
unsigned long lastBlinkTime = 0;    // Last time the LED blinked
constexpr long blinkInterval = 100; // Interval between blinks
int blinkingLED = LED_OFF;          // Use LED_RED or LED_BLUE based on the condition

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

void checkSensorReadings();                       // Function to read and process sensor data
void updateLEDs(bool red, bool green, bool blue); // Function to update the RGB LED
void indicateNormalCondition();                   // Function to indicate normal conditions
void indicateConditionBelowRange();               // Function to indicate condition below range
void indicateConditionAboveRange();               // Function to indicate condition above range
void indicateSensorError();                       // Function to indicate sensor error
void ledBlinking();                               // Function to handle LED blinking and buzzer beeping

// **********************************
// Setup Function
// **********************************
void setup()
{
  Serial.begin(115200); // Initialize serial communication
  dht.begin();          // Initialize the DHT sensor

  // Set RGB LED pins as output
  pinMode(LED_RED, OUTPUT);   // Set LED_RED pin as output
  pinMode(LED_GREEN, OUTPUT); // Set LED_GREEN pin as output
  pinMode(LED_BLUE, OUTPUT);  // Set LED_BLUE pin as output

  // Setup and Attach PWM channels to LED D4
  ledcSetup(LED_D4_CHANNEL, LED_FREQ, LED_RESOLUTION); // Setup LEDC channel for range indicator LED
  ledcAttachPin(LED_D4, LED_D4_CHANNEL);               // Attach range indicator LED to PWM channel

  // Setup and Attach PWM channels to LED D5
  ledcSetup(LED_ERROR_CHANNEL, LED_FREQ, LED_RESOLUTION); // Setup LEDC channel for error indicator LED
  ledcAttachPin(LED_ERROR_INDICATOR, LED_ERROR_CHANNEL);  // Attach error indicator LED to PWM channel

  // Setup and Attach PWM channels to the Piezo Buzzer
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION); // Setup LEDC channel for Piezo Buzzer
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);                 // Attach Piezo Buzzer to PWM channel

  Serial.println("DHT11 sensor monitoring started."); // Inform the user that monitoring has started
}

// **********************************
// Main loop
// **********************************
void loop()
{
  unsigned long currentMillis = millis(); // Get the current time
  // Check if it's time to read the sensor
  if (currentMillis - lastCheckTime >= sensorReadInterval)
  {
    lastCheckTime = currentMillis; // Update the last check time
    checkSensorReadings();         // Read and process sensor data
  }

  ledBlinking(); // Ensure this function is called to handle LED blinking and buzzer beeping
}

// **********************************
// Functions  Definition
// **********************************
void checkSensorReadings()
{
  // Read humidity and temperature from the DHT sensor
  float humidity = dht.readHumidity();            // Read humidity
  float temperatureC = dht.readTemperature();     // Read temperature in Celsius
  float temperatureF = dht.readTemperature(true); // Read temperature in Fahrenheit

  // Check if the readings are valid
  if (isnan(humidity) || isnan(temperatureC))
  {
    indicateSensorError(); // Handle sensor error
    return;
  }

  // Print temperature and humidity
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Temp: ");
  Serial.print(temperatureC);
  Serial.print("C / ");
  Serial.print(temperatureF);
  Serial.println("F");

  // Determine if the readings are within normal ranges
  if (temperatureC >= TEMP_MIN && temperatureC <= TEMP_MAX && humidity >= HUM_MIN && humidity <= HUM_MAX)
  {
    indicateNormalCondition(); // Handle normal condition (RED LED)
  }
  else if (temperatureC < TEMP_MIN || humidity < HUM_MIN)
  {
    indicateConditionBelowRange(); // Handle below normal condition (BLUE LED)
  }
  else
  {
    indicateConditionAboveRange(); // Handle above normal condition (RED LED)
  }
}

void updateLEDs(bool red, bool green, bool blue)
{
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
  digitalWrite(LED_BLUE, blue ? HIGH : LOW);
}

void indicateNormalCondition()
{
  shouldBlink = false;                        // Stop blinking
  updateLEDs(false, true, false);             // Turn on GREEN LED, off others
  ledcWrite(LED_ERROR_CHANNEL, LED_OFF);      // Turn LED D5 solid red
  ledcWrite(BUZZER_CHANNEL, BUZZER_OFF);      // Mute the buzzer
  Serial.println("Current LED Color: GREEN"); // Print current LED color
}

void indicateConditionBelowRange()
{
  shouldBlink = true;
  blinkingLED = LED_BLUE;                             // Indicate that the blue LED should blink
  Serial.println("Current LED Color: BLUE Blinking"); // Updated print statement
}

void indicateConditionAboveRange()
{
  shouldBlink = true;
  blinkingLED = LED_RED;                             // Indicate that the red LED should blink
  Serial.println("Current LED Color: RED Blinking"); // Updated print statement
}

void indicateSensorError()
{
  shouldBlink = false;                           // Stop blinking
  updateLEDs(false, false, false);               // Turn off all LEDs
  ledcWrite(LED_ERROR_CHANNEL, LED_ON);          // Turn LED D5 solid red
  ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME_HALF); // Set buzzer to half volume
  Serial.println("Sensor Error!");               // Inform the user
}

void ledBlinking()
{
  if (!shouldBlink)
    return; // Exit if blinking is not enabled

  unsigned long currentMillis = millis();             // Get current time
  if (currentMillis - lastBlinkTime >= blinkInterval) // Check if it's time to toggle the LED state
  {
    lastBlinkTime = currentMillis; // Update the last blink time
    static bool ledState = false;  // Toggle state variable
    ledState = !ledState;          // Toggle the state

    // Determine which LED to blink based on the condition
    if (blinkingLED == LED_BLUE)
    {
      updateLEDs(false, false, ledState); // Blink Blue LED for below range
    }
    else if (blinkingLED == LED_RED)
    {
      updateLEDs(ledState, false, false); // Blink Red LED for above range
    }

    // Synchronize the buzzer beeping with LED blinking
    ledcWrite(BUZZER_CHANNEL, ledState ? BUZZER_VOLUME_HALF : LED_OFF);
  }
}
