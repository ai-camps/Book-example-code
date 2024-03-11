// **********************************
// Created by: ESP32 Coding Assistant
// Creation Date: 2024-03-011
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
// - updateRGBLEDs()
// - connectToWiFi()
// - pingHost()
// - initNTP()
// Security Considerations:
// - Ensure stable power supply to prevent erroneous readings.
// - Avoid exposing the sensor to extreme conditions beyond its operating range.
// Testing and Validation Approach:
// - Test under known temperature and humidity conditions to validate sensor readings.
// - Simulate error conditions by disconnecting the sensor.
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>          // Include the Arduino base library
#include "DHT.h"              // Include the library for the DHT sensor
#include <WiFi.h>             // Include the WiFi library
#include <ESP32Ping.h>        // Include the Ping library
#include <WiFiClientSecure.h> // Include the WiFiClientSecure library
#include <time.h>             // Include the time library

// **********************************
// Constants Declaration
// **********************************

// DHT11 Sensor Pins and Settings
constexpr int DHTPIN = 2;      // Pin connected to the DHT11 data pin
constexpr int DHTTYPE = DHT11; // Specify DHT11 type

// Data RGB LED Pins
constexpr int DATA_LED_ABOVE_RED = 8;    // Pin for the red component of RGB LED
constexpr int DATA_LED_NORMAL_GREEN = 9; // Pin for the green component of RGB LED
constexpr int DATA_LED_BELOW_BLUE = 4;   // Pin for the blue component of RGB LED

// LED D4 and D5 Pins and PWM settings
constexpr int SYS_LED_D4 = 12;        // Pin to LED D4, used for wifi connection failure indication
constexpr int SYS_LED_D5 = 13;        // Pin to LED D5, used for sensor reading error indication
constexpr int SYS_LED_D4_CHANNEL = 0; // LEDC channel for LED D4
constexpr int SYS_LED_D5_CHANNEL = 1; // LEDC channel for LED D5
constexpr int SYS_LED_FREQ = 5000;    // Frequency for LED PWM
constexpr int SYS_LED_RESOLUTION = 8; // Resolution for LED PWM (8-bit = 0-255)
constexpr int SYS_LED_ON = 255;       // LED ON state
constexpr int SYS_LED_OFF = 0;        // LED OFF state

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

// WiFi and Ping settings
constexpr char *ssid = WIFI_SSID;         // WiFi SSID
constexpr char *password = WIFI_PASSWORD; // WiFi password
constexpr char *host = PING_HOST;         // Host to ping

// NTP settings
const char *ntpServer = NTP_SERVER; // NTP server

// Sensor reading interval
constexpr unsigned long sensorReadInterval = 3000; // Interval between sensor readings
unsigned long lastCheckTime = 0;                   // Last sensor check time

// Declare the error count variable
int sensorErrorCount = 0;

// Blinking control variables
bool shouldBlink = false;           // Flag for LED blinking state
unsigned long lastBlinkTime = 0;    // Last time the LED blinked
constexpr long blinkInterval = 100; // Interval between blinks
int blinkingLED = -1;               // Assuming -1 as an invalid value indicating no LED is set for blinking.

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

// Declare functions
void checkSensorReadings();                          // Function to read and process sensor data
void updateRGBLEDs(bool red, bool green, bool blue); // Function to update the RGB LED
void indicateNormalCondition();                      // Function to indicate normal conditions
void indicateConditionBelowRange();                  // Function to indicate condition below range
void indicateConditionAboveRange();                  // Function to indicate condition above range
void indicateSensorError();                          // Function to indicate sensor error
void ledBlinking();                                  // Function to handle LED blinking and buzzer beeping
void connectToWiFi();                                // Function to connect to WiFi
void pingHost();                                     // Function to ping a host
void initNTP();                                      // Function to initialize NTP
void printLocalTime();                               // Function to print local time

// **********************************
// Setup Function
// **********************************
void setup()
{
    Serial.begin(115200); // Initialize serial communication
    dht.begin();          // Initialize the DHT sensor

    // Set Data LED pins as output
    pinMode(DATA_LED_ABOVE_RED, OUTPUT);    // Set LED RED pin as output
    pinMode(DATA_LED_NORMAL_GREEN, OUTPUT); // Set LED GREEN pin as output
    pinMode(DATA_LED_BELOW_BLUE, OUTPUT);   // Set LED BLUE pin as output

    // Setup and Attach PWM channels to System LED D4
    ledcSetup(SYS_LED_D4_CHANNEL, SYS_LED_FREQ, SYS_LED_RESOLUTION); // Setup LEDC channel for LED D4
    ledcAttachPin(SYS_LED_D4, SYS_LED_D4_CHANNEL);                   // Attach LED D4 to PWM channel

    // Setup and Attach PWM channels to System LED D5
    ledcSetup(SYS_LED_D5_CHANNEL, SYS_LED_FREQ, SYS_LED_RESOLUTION); // Setup LEDC channel for LED D5
    ledcAttachPin(SYS_LED_D5, SYS_LED_D5_CHANNEL);                   // Attach LED D5 to PWM channel

    // Setup and Attach PWM channels to the Piezo Buzzer
    ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION); // Setup LEDC channel for Piezo Buzzer
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);                 // Attach Piezo Buzzer to PWM channel

    Serial.println("DHT11 sensor monitoring started."); // Inform the user that monitoring has started

    connectToWiFi(); // Connect to WiFi
    pingHost();      // Ping the host
    initNTP();       // Initialize NTP
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

    // Reset sensor error count on successful reading
    sensorErrorCount = 0;

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

void updateRGBLEDs(bool red, bool green, bool blue)
{
    digitalWrite(DATA_LED_ABOVE_RED, red ? HIGH : LOW);
    digitalWrite(DATA_LED_NORMAL_GREEN, green ? HIGH : LOW);
    digitalWrite(DATA_LED_BELOW_BLUE, blue ? HIGH : LOW);
}

void indicateNormalCondition()
{
    shouldBlink = false;                        // Stop blinking
    updateRGBLEDs(false, true, false);          // Turn on GREEN LED, off others
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_OFF); // Turn LED D5 solid red
    ledcWrite(BUZZER_CHANNEL, BUZZER_OFF);      // Mute the buzzer
    Serial.println("Current LED Color: GREEN"); // Print current LED color
}

void indicateConditionBelowRange()
{
    shouldBlink = true;
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_OFF);         // Turn LED D5 solid red
    blinkingLED = DATA_LED_BELOW_BLUE;                  // Indicate that the blue LED should blink
    Serial.println("Current LED Color: BLUE Blinking"); // Updated print statement
}

void indicateConditionAboveRange()
{
    shouldBlink = true;
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_OFF);        // Turn LED D5 solid red
    blinkingLED = DATA_LED_ABOVE_RED;                  // Indicate that the red LED should blink
    Serial.println("Current LED Color: RED Blinking"); // Updated print statement
}

void indicateSensorError()
{
    sensorErrorCount++; // Increment the sensor error count

    // Check if the error count has reached 3
    if (sensorErrorCount >= 3)
    {
        Serial.println("Maximum sensor error retries reached. Rebooting...");
        delay(1000);   // Short delay to allow the message to be sent before rebooting
        ESP.restart(); // Reboot the ESP device
    }
    shouldBlink = false;                           // Stop blinking
    updateRGBLEDs(false, false, false);            // Turn off all LEDs
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_ON);     // Turn LED D5 solid red
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
        if (blinkingLED == DATA_LED_BELOW_BLUE)
        {
            updateRGBLEDs(false, false, ledState); // Blink Blue LED for below range
        }
        else if (blinkingLED == DATA_LED_ABOVE_RED)
        {
            updateRGBLEDs(ledState, false, false); // Blink Red LED for above range
        }

        // Synchronize the buzzer beeping with LED blinking
        ledcWrite(BUZZER_CHANNEL, ledState ? BUZZER_VOLUME_HALF : BUZZER_OFF);
    }
}

void connectToWiFi()
{
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);        // Set WiFi mode to station to connect to a WiFi network
    WiFi.begin(ssid, password); // Start the connection process

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 3)
    {
        delay(5000); // Wait 5 seconds between attempts
        attempts++;
        Serial.print("Attempt ");
        Serial.print(attempts);
        Serial.println(": Trying to connect to WiFi...");

        if (attempts >= 1)
        {
            // Immediately turn on LED D4 solidly after the first failed attempt
            ledcWrite(SYS_LED_D4_CHANNEL, SYS_LED_ON);
            ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME_HALF); // Set buzzer to half volume
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        // If connected successfully, turn off LED D4 and print the IP address and RSSI
        Serial.println("Connected to WiFi successfully!");
        Serial.print("WiFi SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        long rssi = WiFi.RSSI();
        Serial.print("RSSI: ");
        Serial.print(rssi);
        Serial.println(" dBm");
        ledcWrite(SYS_LED_D4_CHANNEL, SYS_LED_OFF); // Turn off LED D4 when WiFi is connected
        ledcWrite(BUZZER_CHANNEL, BUZZER_OFF);      // Mute the buzzer when WiFi is connected
    }
    else
    {
        // If connection failed after 3 attempts, keep LED D4 on to indicate failure
        Serial.println("Failed to connect to WiFi. Rebooting...");
        delay(5000);   // Delay to allow the serial message to be sent before rebooting
        ESP.restart(); // Reboot the ESP32
    }
}

void pingHost()
{
    Serial.print("Pinging host: " + String(host) + "...");

    if (Ping.ping(host))
    {
        Serial.println("Ping successful.");
    }
    else
    {
        Serial.println("Ping failed.");
    }
}

void initNTP()
{
    configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, ntpServer); // Use GMT and DST offsets directly

    Serial.println("Initializing NTP...");

    // Wait until time has been retrieved
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Waiting for NTP time sync...");
        delay(1000);
    }

    // Print the current time
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "Current time: %A, %B %d %Y %H:%M:%S", &timeinfo);
    Serial.println(timeStr);
}

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "Current time: %A, %B %d %Y %H:%M:%S");
}
