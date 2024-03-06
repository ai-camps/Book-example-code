// **********************************
// Created by: ESP32-C3 Coding Assistant
// Creation Date: 2024-03-07
// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This program is designed to work with the ESP32-C3 module to read temperature and humidity data from a DHT11 sensor,
// and provide visual and audible alerts based on predefined conditions. It visually indicates the range of the
// temperature and humidity data using LEDs and signals any system errors or out-of-range conditions.
//
// Requirement Summary:
// 1. Connect a DHT11 sensor to Pin 2 (IO02) to read temperature and humidity.
// 2. Use a Piezo Buzzer connected to Pin 3 (IO11) for audible alerts.
// 3. Visual indications through LEDs for sensor data range and errors.
//
// Hardware Connection:
// - DHT11 Data Pin connected to GPIO2 (IO02).
// - Piezo Buzzer connected to GPIO11 (IO11).
// - LED for Temperature & Humidity Range (Visual indication) connected to GPIO12 (IO12).
// - LED for Error Indication connected to GPIO13 (IO13).
//
// New Created Function/Class:
// - DHT sensor handling for temperature and humidity reading.
// - Non-blocking LED control for visual feedback.
//
// Security Considerations:
// Implement error handling for the sensor to ensure reliability.
// Validate sensor data to prevent incorrect alerts.
//
// Testing and Validation Approach:
// The code should be tested with varying environmental conditions to ensure accurate temperature and humidity readings,
// and appropriate LED and Buzzer responses to those readings and error conditions.
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h> // Standard Arduino library
#include "DHT.h"     // DHT sensor library

// **********************************
// Constants Declaration
// **********************************
#define DHTPIN 2               // Pin connected to the DHT11 data pin
#define DHTTYPE DHT11          // Specify DHT11 type
#define BUZZER_PIN 11          // Pin connected to the Piezo Buzzer
#define LED_RANGE_INDICATOR 12 // LED for temperature & humidity range indication
#define LED_ERROR_INDICATOR 13 // LED for error indication

// Temperature and Humidity Ranges
const float TEMP_MIN = 10.0; // Minimum normal temperature
const float TEMP_MAX = 25.0; // Maximum normal temperature
const float HUM_MIN = 10.0;  // Minimum normal humidity
const float HUM_MAX = 60.0;  // Maximum normal humidity

// **********************************
// Variables Declaration
// **********************************
DHT dht(DHTPIN, DHTTYPE);                // DHT sensor object
unsigned long sensorReadInterval = 5000; // Time interval for sensor readings in milliseconds
unsigned long lastCheckTime = 0;         // Time of the last sensor reading

// Blink control variables
bool ledIndicatorState = LOW;    // Indicates whether the LED should be blinking.
unsigned long lastBlinkTime = 0; // Timestamp of the last LED blink toggle.
const long blinkInterval = 100;  // Interval for LED blinking in milliseconds.
bool ledState = LOW;             // Current state of the LED, initialized to OFF.

// **********************************
// Functions or Class Declaration
// **********************************
void manageBlinking(unsigned long currentMillis); // Manages non-blocking blinking of the LED.
void checkSensorReadings();                       // Reads sensor data and updates indicators accordingly.
void indicateNormalCondition();                   // Handles indicators for normal sensor readings.
void indicateAbnormalCondition();                 // Handles indicators for out-of-range sensor readings.
void indicateSensorError();                       // Handles indicators for sensor errors.

// **********************************
// Setup Function
// **********************************
void setup()
{
  Serial.begin(115200);
  pinMode(LED_RANGE_INDICATOR, OUTPUT); // Set LED pin as output
  pinMode(LED_ERROR_INDICATOR, OUTPUT); // Set LED pin as output
  dht.begin();                          // Start DHT sensor
  Serial.println("DHT11 sensor monitoring started.");
}

// **********************************
// Main loop
// **********************************
// Main program loop, which runs repeatedly after setup().
void loop()
{
  unsigned long currentMillis = millis(); // Get the current time in milliseconds.

  // Check if it's time to read the sensor again.
  if (currentMillis - lastCheckTime >= sensorReadInterval)
  {
    lastCheckTime = currentMillis; // Update the last check time.
    checkSensorReadings();         // Read and process sensor readings.
  }

  // Call manageBlinking to handle the LED blinking logic.
  manageBlinking(currentMillis);
}

// Reads humidity and temperature from the DHT sensor, displays them, and updates indicator LEDs.
void checkSensorReadings()
{
  float humidity = dht.readHumidity();            // Read humidity value from sensor.
  float temperatureC = dht.readTemperature();     // Read temperature in Celsius.
  float temperatureF = temperatureC * 9 / 5 + 32; // Convert Celsius to Fahrenheit.
  delay(100);                                     // Adds a small delay to stabilize sensor readings (optional).

  if (isnan(humidity) || isnan(temperatureC))
  {                        // Check if readings are valid.
    indicateSensorError(); // Handle sensor error.
    return;
  }

  // Display the humidity and temperature readings in both Celsius and Fahrenheit.
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Temp: ");
  Serial.print(temperatureC);
  Serial.print("C / ");
  Serial.print(temperatureF);
  Serial.println("F");

  // Update indicators based on whether readings are within the normal range.
  if (temperatureC >= TEMP_MIN && temperatureC <= TEMP_MAX && humidity >= HUM_MIN && humidity <= HUM_MAX)
  {
    indicateNormalCondition(); // Handle normal condition.
  }
  else
  {
    indicateAbnormalCondition(); // Handle abnormal condition.
  }
}

// Updates indicators to reflect normal sensor reading conditions.
void indicateNormalCondition()
{
  digitalWrite(LED_RANGE_INDICATOR, LOW); // Turn off range indicator LED.
  digitalWrite(LED_ERROR_INDICATOR, LOW); // Turn off error indicator LED.
  noTone(BUZZER_PIN);                     // Turn off the buzzer.
  ledIndicatorState = false;              // Stop blinking.
}

// Updates indicators to reflect abnormal sensor reading conditions and starts LED blinking.
void indicateAbnormalCondition()
{
  digitalWrite(LED_ERROR_INDICATOR, LOW); // Ensure the error indicator is off.
  ledIndicatorState = true;               // Start LED blinking.
}

// Updates indicators to reflect a sensor error condition.
void indicateSensorError()
{
  digitalWrite(LED_RANGE_INDICATOR, LOW);  // Ensure the range indicator is off.
  digitalWrite(LED_ERROR_INDICATOR, HIGH); // Turn on the error indicator LED.
  tone(BUZZER_PIN, 2000);                   // Emit a high-pitch tone to indicate an error.
  ledIndicatorState = false;               // Stop LED blinking
  // Print "Sensor Error" message to the serial monitor
  Serial.println("Sensor Error!");
}

// Manages the non-blocking blinking of the LED and synchronizes the buzzer.
void manageBlinking(unsigned long currentMillis)
{
  if (ledIndicatorState && (currentMillis - lastBlinkTime >= blinkInterval))
  {
    lastBlinkTime = currentMillis; // Update the last blink time.

    ledState = !ledState;                        // Toggle the LED state.
    digitalWrite(LED_RANGE_INDICATOR, ledState); // Update the LED's physical state.

    // Sync the buzzer with the LED state.
    if (ledState == HIGH)
    {
      tone(BUZZER_PIN, 1000); // Turn on the buzzer with a specified frequency when the LED is on.
    }
    else
    {
      noTone(BUZZER_PIN); // Turn off the buzzer when the LED is off.
    }
  }
  else if (!ledIndicatorState)
  {
    // Ensure the buzzer is off when the LED is not supposed to blink.
    noTone(BUZZER_PIN);
  }
}
