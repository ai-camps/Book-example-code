// **********************************
// Created by: ESP32 Coding Assistant
// Creation Date: 2024-03-07
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
// - LED D4 (normal condition indicator) -> IO12
// - LED D5 (error indicator) -> IO13
// New Created Function/Class:
// - checkSensorReadings()
// - indicateNormalCondition()
// - indicateAbnormalCondition()
// - indicateSensorError()
// - ledBlinking
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
#define DHTPIN 2      // Pin connected to the DHT11 data pin
#define DHTTYPE DHT11 // Specify DHT11 type
#define BUZZER_PIN 11 // Pin connected to the Piezo Buzzer

#define LED_RANGE_INDICATOR 12 // LED for temperature & humidity range indication
#define LED_ERROR_INDICATOR 13 // LED for error indication

#define LED_RANGE_CHANNEL 0 // LEDC channel for range indicator
#define LED_ERROR_CHANNEL 1 // LEDC channel for error indicator
#define LED_FREQ 5000       // Frequency for LED PWM
#define LED_RESOLUTION 8    // Resolution for LED PWM (8-bit = 0-255)
#define LED_ON 255
#define LED_OFF 0

#define BUZZER_CHANNEL 2     // LEDC channel for Piezo Buzzer
#define BUZZER_FREQ 2000     // Frequency for Buzzer PWM
#define BUZZER_RESOLUTION 10 // Resolution for Buzzer PWM (10-bit = 0-1023)
#define BUZZER_VOLUME_HALF 512
#define BUZZER_OFF 0

// **********************************
// Variables Declaration
// **********************************
const float TEMP_MIN = 10.0; // Minimum normal temperature
const float TEMP_MAX = 25.0; // Maximum normal temperature
const float HUM_MIN = 10.0;  // Minimum normal humidity
const float HUM_MAX = 80.0;  // Maximum normal humidity

// Timing and control variables
unsigned long sensorReadInterval = 3000; // Interval between sensor readings
unsigned long lastCheckTime = 0;         // Last sensor check time

bool isBlinking = false;         // Flag for LED blinking state
unsigned long lastBlinkTime = 0; // Last time the LED blinked
const long blinkInterval = 100;  // Interval between blinks

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

struct SensorState
{
  bool isBlinking;
  unsigned long lastBlinkTime;
};

SensorState sensorState = {false, 0};

// **********************************
// Functions  Declaration
// **********************************
void checkSensorReadings();
void indicateNormalCondition();
void indicateAbnormalCondition();
void indicateSensorError();
void ledBlinking();

// **********************************
// Setup Function
// **********************************
void setup()
{
  Serial.begin(115200); // Initialize serial communication
  dht.begin();          // Initialize the DHT sensor

  // Setup PWM for LEDs and buzzer
  ledcSetup(LED_RANGE_CHANNEL, LED_FREQ, LED_RESOLUTION);
  ledcSetup(LED_ERROR_CHANNEL, LED_FREQ, LED_RESOLUTION);
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION);

  // Attach PWM channels to GPIO pins
  ledcAttachPin(LED_RANGE_INDICATOR, LED_RANGE_CHANNEL);
  ledcAttachPin(LED_ERROR_INDICATOR, LED_ERROR_CHANNEL);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

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
  ledBlinking(); // Handle LED blinking and buzzer beeping
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

  // Print both Celsius and Fahrenheit temperatures
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
    indicateNormalCondition(); // Handle normal condition
  }
  else
  {
    indicateAbnormalCondition(); // Handle abnormal condition
  }
}

void indicateNormalCondition()
{
  // Stop blinking and beeping for normal conditions
  sensorState.isBlinking = false;
  ledcWrite(LED_RANGE_CHANNEL, LED_OFF); // Turn off range indicator LED
  ledcWrite(LED_ERROR_CHANNEL, LED_OFF); // Turn off error indicator LED
  ledcWrite(BUZZER_CHANNEL, BUZZER_OFF); // Mute the buzzer
}

void indicateAbnormalCondition()
{
  // Start blinking and set buzzer for abnormal conditions
  sensorState.isBlinking = true;
  ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME_HALF);  // Set buzzer to half volume (value can be adjusted)
  Serial.println("Abnormal Condition Detected!"); // Inform the user
}

void indicateSensorError()
{
  // Handle sensor error: stop blinking, set error indicator, and beep continuously
  sensorState.isBlinking = false;                // Stop blinking
  ledcWrite(LED_ERROR_CHANNEL, LED_ON);          // Turn LED D5 solid red
  ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME_HALF); // Adjust 512 to your desired buzzer volume
  Serial.println("Sensor Error!");               // Inform the user
}

void ledBlinking()
{
  // Handle blinking of the range indicator LED and buzzer beeping
  if (!sensorState.isBlinking)
    return; // Exit if blinking is not enabled

  unsigned long currentMillis = millis(); // Get current time
  // Check if it's time to toggle the LED and buzzer state
  if (currentMillis - lastBlinkTime >= blinkInterval)
  {
    lastBlinkTime = currentMillis;
    static bool ledState = false; // Toggle state variable
    ledState = !ledState;         // Toggle the state
    // Toggle the range indicator LED for abnormal conditions
    ledcWrite(LED_RANGE_CHANNEL, ledState ? LED_ON : LED_OFF);
    // Synchronize the buzzer beeping with LED blinking
    ledcWrite(BUZZER_CHANNEL, ledState ? BUZZER_VOLUME_HALF : LED_OFF); // Adjust the value 512 as needed for desired buzzer volume
  }
}
