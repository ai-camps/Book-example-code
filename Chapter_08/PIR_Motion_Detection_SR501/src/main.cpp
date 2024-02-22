#include <Arduino.h>
#include "Adafruit_NeoPixel.h" // For controlling the built-in RGB LED.

// Pin Definitions
#define SENSOR_PIN 4  // HC-SR501 OUT pin connected to GPIO 4
#define BUZZER_PIN 5  // Piezo Buzzer connected to GPIO 5
#define LED_PIN 8     // Built-in RGB LED on GPIO 8 (adjust if your board has a different pin)
#define STRIP_COUNT 1 // Number of the strip

int pixelIndex = 0; // Index of the pixel to be updated

// Color Definitions
struct Colors
{
  static const uint32_t redColor = 0xFF0000;   // Color value for red
  static const uint32_t greenColor = 0x00FF00; // Color value for green
  static const uint32_t blueColor = 0x0000FF;  // Color value for blue
  static const uint32_t offColor = 0x000000;   // Color value for LED off (black/no color)
};
// Class to control the LED based on flame detection.
class LEDController
{
private:
  Adafruit_NeoPixel strip;                 // Object to control the NeoPixel LED strip.
  unsigned long lastBlinkTime = 0;         // Tracks the last time the LED blinked for non-blocking delay.
  const unsigned long blinkInterval = 500; // Time in milliseconds between blinks.
  bool ledState = false;                   // Current state of the LED, toggled for blinking.

public:
  // Constructor initializes the NeoPixel strip object.
  LEDController(uint8_t pin, uint8_t count) : strip(count, pin, NEO_GRB + NEO_KHZ800) {}

  // Method to initialize the LED strip.
  void begin()
  {
    strip.begin(); // Initialize the LED strip.
    strip.show();  // Turn off all LEDs at start.
  }

  // Method to set the LED strip to a specific color.
  void setColor(uint32_t color)
  {
    strip.setPixelColor(pixelIndex, color); // Fill the strip with the specified color.
    strip.show();                           // Update the strip to show the new color.
  }

  // Method to blink the LED strip at a specified color.
  void blinkLED(unsigned long currentMillis)
  {
    if (currentMillis - lastBlinkTime >= blinkInterval)
    {
      ledState = !ledState;                                                            // Toggle the state of the LED.
      strip.setPixelColor(pixelIndex, ledState ? Colors::redColor : Colors::offColor); // Set the color or turn off based on the toggled state.
      strip.show();                                                                    // Update the strip to show the change.
      lastBlinkTime = currentMillis;                                                   // Update the last blink time.
    }
  }
};

class BuzzerController
{
  int buzzerPin;

public:
  BuzzerController(int pin) : buzzerPin(pin) {}

  void begin()
  {
    pinMode(buzzerPin, OUTPUT);
  }

  void beep(int durationMs, int frequency)
  {
    tone(buzzerPin, frequency, durationMs);
  }
};

LEDController led(LED_PIN, STRIP_COUNT);
BuzzerController buzzer(BUZZER_PIN);

void setup()
{
  Serial.begin(115200);
  pinMode(SENSOR_PIN, INPUT);
  led.begin();
  buzzer.begin();
}

/* void loop()
{
  static unsigned long lastCheckTime = 0;
  const unsigned long checkInterval = 1000;     // Check every 1 seconds

  unsigned long currentMillis = millis();
  if (currentMillis - lastCheckTime >= checkInterval)
  {
    lastCheckTime = currentMillis;
    int motionDetected = digitalRead(SENSOR_PIN);

    if (motionDetected)
    {
      Serial.println("Motion is detected!");
      led.blinkLED(currentMillis);
      buzzer.beep(200, 200); // Faster beep with a higher pitch
    }
    else
    {
      Serial.println("No motion is detected!");
      led.setColor(Colors::redColor);      // Set to green only if no motion was detected and not previously detected
    }
  }
} */

bool previousMotionDetected = false; // Add this line at the global scope

void loop()
{
  static unsigned long lastCheckTime = 0;
  const unsigned long checkInterval = 1000; // Check every 1 seconds

  unsigned long currentMillis = millis();
  if (currentMillis - lastCheckTime >= checkInterval)
  {
    lastCheckTime = currentMillis;
    bool motionDetected = digitalRead(SENSOR_PIN);

    // Only change LED state if motion detection state changes
    if (motionDetected != previousMotionDetected)
    {
      if (motionDetected)
      {
        Serial.println("Motion is detected!");
        led.blinkLED(currentMillis);
        buzzer.beep(200, 200); // Faster beep with a higher pitch
      }
      else
      {
        Serial.println("No motion is detected!");
        led.setColor(Colors::greenColor); // Set to red if no motion was detected
      }
      previousMotionDetected = motionDetected;
    }
  }
}