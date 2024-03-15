// **********************************
// Created by: ESP32 Coding Assistant
// Creation Date: 2024-03-12
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
// - connectAWS()
// - mqttPublishMessage()
// Security Considerations:
// - Ensure stable power supply to prevent erroneous readings.
// - Avoid exposing the sensor to extreme conditions beyond its operating range.
// Testing and Validation Approach:
// - Test under known temperature and humidity conditions to validate sensor readings.
// - Simulate error conditions by disconnecting the sensor.
// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>           // Include the Arduino base library
#include "DHT.h"               // Include the library for the DHT sensor
#include <WiFi.h>              // Include the WiFi library
#include <ESP32Ping.h>         // Include the Ping library
#include <WiFiClientSecure.h>  // Include the WiFiClientSecure library
#include <time.h>              // Include the time library
#include <ArduinoJson.h>       // Include the ArduinoJson library
#include "SecureCredentials.h" // Include the secrets file
#include <Update.h>            // Include the Update library
#include <PubSubClient.h>      // Include the PubSubClient library
#include "HardwareInfo.h"      // Include the HardwareInfo class

// **********************************
// Constants Declaration
// **********************************

// * ESP32 Reboot Delay
constexpr int ESP32_REBOOT_DELAY_MS = 5000; // Delay before rebooting the ESP32

// * DHT11 Sensor Pins and Settings
constexpr int DHT_PIN = 2;      // Pin connected to the DHT11 data pin
constexpr int DHT_TYPE = DHT11; // Specify DHT11 type

// * Data RGB LED Pins
constexpr int DATA_LED_ABOVE_RED = 8;    // Pin for the red component of RGB LED
constexpr int DATA_LED_NORMAL_GREEN = 9; // Pin for the green component of RGB LED
constexpr int DATA_LED_BELOW_BLUE = 4;   // Pin for the blue component of RGB LED

// * ESP32 LED D4 and D5 Pins and PWM settings
constexpr int SYS_LED_D4 = 12;        // Pin to LED D4, used for wifi connection failure indication
constexpr int SYS_LED_D5 = 13;        // Pin to LED D5, used for sensor reading error indication
constexpr int SYS_LED_D4_CHANNEL = 0; // LEDC channel for LED D4
constexpr int SYS_LED_D5_CHANNEL = 1; // LEDC channel for LED D5
constexpr int SYS_LED_FREQ = 5000;    // Frequency for LED PWM
constexpr int SYS_LED_RESOLUTION = 8; // Resolution for LED PWM (8-bit = 0-255)
constexpr int SYS_LED_ON = 255;       // LED ON state
constexpr int SYS_LED_OFF = 0;        // LED OFF state

// * Buzzer Pins and PWM settings
constexpr int BUZZER_PIN = 11;          // Pin connected to the Piezo Buzzer
constexpr int BUZZER_CHANNEL = 2;       // LEDC channel for Piezo Buzzer
constexpr int BUZZER_FREQ = 2000;       // Frequency for Buzzer PWM
constexpr int BUZZER_RESOLUTION = 10;   // Resolution for Buzzer PWM (10-bit = 0-1023)
constexpr int BUZZER_VOLUME_HALF = 512; // Half volume for the buzzer
constexpr int BUZZER_OFF = 0;           // Turn off the buzzer

// * Normal data ranges for temperature and humidity
constexpr float TEMP_MIN = 10.0; // Minimum normal temperature
constexpr float TEMP_MAX = 25.0; // Maximum normal temperature
constexpr float HUM_MIN = 10.0;  // Minimum normal humidity
constexpr float HUM_MAX = 80.0;  // Maximum normal humidity

// * DHT11 reading interval
constexpr unsigned long SENSOR_READ_INTERVAL = 3000; // Interval between sensor readings
unsigned long lastCheckTime = 0;                     // Last sensor check time
constexpr int MAX_SENSOR_ERROR_RETRIES = 3;          // Maximum number of sensor error retries
int sensorErrorCount = 0;                            // Counter for sensor error retries

// * Data ranges for condition status
enum ConditionStatus
{
    Normal,
    BelowNormal,
    AboveNormal,
    Error // Optional, if you want to handle sensor error status
};

ConditionStatus currentCondition = Error; // Default to Error until the first successful reading

// * Blinking control variables
bool shouldBlink = false;                        // Flag for LED blinking state
unsigned long lastBlinkTime = 0;                 // Last time the LED blinked
constexpr long DATA_LED_BLINK_INTERVAL_MS = 100; // Interval between blinks
int blinkingLED = -1;                            // Assuming -1 as an invalid value indicating no LED is set for blinking.

// * WiFi, Ping and NTP Sync settings
const char *ssid = WIFI_SSID;                               // WiFi SSID
const char *password = WIFI_PASSWORD;                       // WiFi password
const char *host = PING_HOST;                               // Host to ping
constexpr int MAX_WIFI_CONNECT_ATTEMPTS = 3;                // Maximum number of WiFi connection attempts
constexpr unsigned long WIFI_CONNECT_RETRY_DELAY_MS = 5000; // Delay between WiFi connection attempts
const char *ntpServer = NTP_SERVER;                         // NTP server
constexpr unsigned long NTP_SYNC_DELAY_MS = 1000;           // Delay between NTP time sync attempts

// * AWS IoT Core access settings
WiFiClientSecure net = WiFiClientSecure();              // Create a WiFiClientSecure to handle the MQTT connection
PubSubClient mqttClient(net);                           // Create a PubSubClient to handle the MQTT connection
constexpr unsigned long MQTT_RECONNECT_DELAY_MS = 3000; // Delay between reconnect attempts
String deviceID;                                        // Device ID for the AWS IoT Core
String AWS_IOT_PUBLISH_TOPIC;                           // MQTT topic to publish messages

// * Initialize the DHT11
DHT dht(DHT_PIN, DHT_TYPE); // Initialize DHT sensor

// * Declare functions
void checkSensorReadings(float &humidity, float &temperatureC, float &temperatureF); // Function to read and process sensor data
void updateRGBLEDs(bool red, bool green, bool blue);                                 // Function to update the RGB LED
void indicateNormalCondition();                                                      // Function to indicate normal conditions
void indicateConditionBelowRange();                                                  // Function to indicate condition below range
void indicateConditionAboveRange();                                                  // Function to indicate condition above range
void indicateSensorError();                                                          // Function to indicate sensor error
void ledBlinking();                                                                  // Function to handle LED blinking and buzzer beeping
void connectToWiFi();                                                                // Function to connect to WiFi
void pingHost();                                                                     // Function to ping a host
void initNTP();                                                                      // Function to initialize NTP
void printLocalTime();                                                               // Function to print local time
void connectAWS();                                                                   // Function to connect to AWS IoT Core
void mqttPublishMessage(float humidity, float temperatureC, float temperatureF);     // Function to publish message to AWS IoT Core

// **********************************
// & Setup Function
// **********************************
void setup()
{
    Serial.begin(115200); // Initialize serial communication

    HardwareInfo::displayHardwareInfo(); // Display hardware information

    deviceID = String(ESP.getEfuseMac(), HEX); // Get the device ID
    AWS_IOT_PUBLISH_TOPIC = deviceID + "/pub"; // Set the MQTT topic to publish messages

    dht.begin(); // Initialize the DHT sensor

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
    connectAWS();    // Connect to AWS IoT Core
}

// **********************************
// & Main loop
// **********************************
void loop()
{
    static float humidity = 0;     // Initialize humidity variable
    static float temperatureC = 0; // Initialize temperature in Celsius variable
    static float temperatureF = 0; // Initialize temperature in Fahrenheit variable

    unsigned long currentMillis = millis();
    if (currentMillis - lastCheckTime >= SENSOR_READ_INTERVAL) // Check if it's time to read the sensor
    {
        lastCheckTime = currentMillis;
        checkSensorReadings(humidity, temperatureC, temperatureF); // Read sensor and update humidity and temperature

        // Only publish if readings are valid (Not NaN)
        if (!isnan(humidity) && !isnan(temperatureC))
        {
            mqttPublishMessage(humidity, temperatureC, temperatureF); // Publish message to AWS IoT Core
        }
    }

    ledBlinking(); // Handle LED blinking and buzzer beeping
}

// **********************************
// & Functions  Definition
// **********************************
void checkSensorReadings(float &humidity, float &temperatureC, float &temperatureF)
{
    humidity = dht.readHumidity();            // Read humidity
    temperatureC = dht.readTemperature();     // Read temperature in Celsius
    temperatureF = dht.readTemperature(true); // Read temperature in Fahrenheit

    // Validation and action based on the read values
    if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) // Check if any of the readings are NaN
    {
        indicateSensorError();    // Indicate sensor error
        currentCondition = Error; // Set current condition to Error
    }
    else
    {
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%, Temp: ");
        Serial.print(temperatureC);
        Serial.print("C / ");
        Serial.print(temperatureF);
        Serial.println("F");

        // Condition indications based on readings
        if (temperatureC >= TEMP_MIN && temperatureC <= TEMP_MAX && humidity >= HUM_MIN && humidity <= HUM_MAX) // Check if readings are within normal range
        {
            indicateNormalCondition(); // Indicate normal condition
            currentCondition = Normal; // Set current condition to Normal
        }
        else if (temperatureC < TEMP_MIN || humidity < HUM_MIN) // Check if readings are below normal range

        {
            indicateConditionBelowRange();  // Indicate condition below range
            currentCondition = BelowNormal; // Set current condition to BelowNormal
        }
        else
        {
            indicateConditionAboveRange();  // Indicate condition above range
            currentCondition = AboveNormal; // Set current condition to AboveNormal
        }
    }
}

void updateRGBLEDs(bool red, bool green, bool blue) // Function to update the RGB LED
{
    digitalWrite(DATA_LED_ABOVE_RED, red ? HIGH : LOW);      // Turn on or off the red LED
    digitalWrite(DATA_LED_NORMAL_GREEN, green ? HIGH : LOW); // Turn on or off the green LED
    digitalWrite(DATA_LED_BELOW_BLUE, blue ? HIGH : LOW);    // Turn on or off the blue LED
}

void indicateNormalCondition() // Function to indicate normal conditions
{
    shouldBlink = false;                        // Stop blinking
    updateRGBLEDs(false, true, false);          // Turn on GREEN LED, off others
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_OFF); // Turn LED D5 off
    ledcWrite(BUZZER_CHANNEL, BUZZER_OFF);      // Mute the buzzer
    Serial.println("Current LED Color: GREEN"); // Print current LED color
}

void indicateConditionBelowRange() // Function to indicate condition below range
{
    shouldBlink = true;                                 // Indicate that the blue LED should blink
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_OFF);         // Turn LED D5 off
    blinkingLED = DATA_LED_BELOW_BLUE;                  // Blink the blue LED
    Serial.println("Current LED Color: BLUE Blinking"); // Updated print statement
}

void indicateConditionAboveRange() // Function to indicate condition above range
{
    shouldBlink = true;                                // Indicate that the red LED should blink
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_OFF);        // Turn LED D5 off
    blinkingLED = DATA_LED_ABOVE_RED;                  // Blink the red LED
    Serial.println("Current LED Color: RED Blinking"); // Updated print statement
}

void indicateSensorError() // Function to indicate sensor error
{
    sensorErrorCount++; // Increment the sensor error count

    // Check if the error count has reached 3
    if (sensorErrorCount >= MAX_SENSOR_ERROR_RETRIES)
    {
        Serial.println("Maximum sensor error retries reached. Rebooting...");
        delay(ESP32_REBOOT_DELAY_MS); // Short delay to allow the message to be sent before rebooting
        ESP.restart();                // Reboot the ESP device
    }
    shouldBlink = false;                           // Stop blinking
    updateRGBLEDs(false, false, false);            // Turn off all LEDs
    ledcWrite(SYS_LED_D5_CHANNEL, SYS_LED_ON);     // Turn LED D5 solid red
    ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME_HALF); // Set buzzer to half volume
    Serial.println("Sensor Error!");               // Print sensor error message
}

void ledBlinking() // Function to handle LED blinking and buzzer beeping
{
    if (!shouldBlink) // Check if blinking is not enabled
        return;       // Exit if blinking is not enabled

    unsigned long currentMillis = millis();                          // Get current time
    if (currentMillis - lastBlinkTime >= DATA_LED_BLINK_INTERVAL_MS) // Check if it's time to toggle the LED state
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

void connectToWiFi() // Function to connect to WiFi
{
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);        // Set WiFi mode to station to connect to a WiFi network
    WiFi.begin(ssid, password); // Start the connection process

    int attempts = 0;                                                             // Initialize the number of connection attempts
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_CONNECT_ATTEMPTS) // Check if WiFi is not connected and the number of attempts is less than the maximum
    {
        delay(WIFI_CONNECT_RETRY_DELAY_MS); // Wait 5 seconds between attempts
        attempts++;
        Serial.print("Attempt ");
        Serial.print(attempts);
        Serial.println(": Trying to connect to WiFi...");

        if (attempts >= 1) // Check if the number of attempts is greater than or equal to 1
        {
            // Immediately turn on LED D4 solidly after the first failed attempt
            ledcWrite(SYS_LED_D4_CHANNEL, SYS_LED_ON);     // Turn on LED D4 when WiFi is not connected
            ledcWrite(BUZZER_CHANNEL, BUZZER_VOLUME_HALF); // Set buzzer to half volume
        }
    }

    if (WiFi.status() == WL_CONNECTED) // Check if WiFi is connected
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
        delay(ESP32_REBOOT_DELAY_MS); // Delay to allow the serial message to be sent before rebooting
        ESP.restart();                // Reboot the ESP32
    }
}

void pingHost() // Function to ping a host
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

void initNTP() // Function to initialize NTP
{
    configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, ntpServer); // Use GMT and DST offsets directly

    Serial.println("Initializing NTP...");
    struct tm timeinfo; // Create a timeinfo struct
    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Waiting for NTP time sync...");
        delay(NTP_SYNC_DELAY_MS);
    }

    // Print the current time
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "Current time: %A, %B %d %Y %H:%M:%S", &timeinfo);
    Serial.println(timeStr);
}

void printLocalTime() // Function to print local time
{
    struct tm timeinfo; // Create a timeinfo struct
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "Current time: %A, %B %d %Y %H:%M:%S");
}

void connectAWS() // Function to connect to AWS IoT Core
{
    // Configure WiFiClientSecure to use the AWS IoT device credentials
    net.setCACert(AWS_ROOT_CA);         // Set the AWS Root CA certificate
    net.setCertificate(AWS_CERT_CRT);   // Set the device certificate
    net.setPrivateKey(AWS_PRIVATE_KEY); // Set the private key

    // Set the AWS IoT endpoint and port
    mqttClient.setServer(AWS_IOT_MQTT_SERVER, AWS_IOT_MQTT_PORT);

    Serial.println("Connecting to AWS IOT Core");

    while (!mqttClient.connect(deviceID.c_str())) // Connect to AWS IoT Core
    {
        Serial.print(".");
        delay(MQTT_RECONNECT_DELAY_MS);
    }

    if (!mqttClient.connected()) // Check if the client is connected
    {
        Serial.println("AWS IoT Core connection is failed!");
        return;
    }
    Serial.println("AWS IoT Core is connected successfully!");
}

void mqttPublishMessage(float humidity, float temperatureC, float temperatureF) // Function to publish message to AWS IoT Core
{
    if (!mqttClient.connected()) // Check if the client is connected
    {
        connectAWS(); // Connect to AWS IoT Core if not connected
    }

    String conditionStr;
    switch (currentCondition)
    {
    case Normal:
        conditionStr = "Normal";
        break;
    case BelowNormal:
        conditionStr = "Below Normal";
        break;
    case AboveNormal:
        conditionStr = "Above Normal";
        break;
    case Error:
    default:
        conditionStr = "Error"; // Handle the error or uninitialized state
        break;
    }

    DynamicJsonDocument doc(ESP.getMaxAllocHeap()); // Create a JSON document with the maximum heap size
    doc["deviceType"] = "Sensor";
    doc["deviceFunction"] = "Temperature and Humidity";
    doc["deviceModel"] = "DHT11";
    doc["deviceID"] = String(ESP.getEfuseMac(), HEX);
    doc["temp_C"] = temperatureC;
    doc["temp_F"] = round(temperatureF);
    doc["humidity"] = humidity;
    doc["status"] = conditionStr;
    doc["SSID"] = WiFi.SSID();
    doc["IP"] = WiFi.localIP().toString();
    doc["RSSI"] = WiFi.RSSI();

    // Print out the JSON document to the Serial Monitor
    Serial.print("Publishing message: ");
    serializeJson(doc, Serial); // Serialize the JSON document
    Serial.println();

    // Determine buffer size
    size_t jsonSize = measureJson(doc) + 1; // +1 for null terminator
    Serial.print("Calculated JSON buffer size: ");
    Serial.println(jsonSize); // Print the buffer size

    char jsonBuffer[jsonSize];
    serializeJson(doc, jsonBuffer, sizeof(jsonBuffer));

    if (!mqttClient.publish(AWS_IOT_PUBLISH_TOPIC.c_str(), jsonBuffer))
    {
        Serial.println("Publish failed");
    }
    else
    {
        Serial.println("Publish succeeded");
    }
}
