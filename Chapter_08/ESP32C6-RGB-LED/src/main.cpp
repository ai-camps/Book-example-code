// **********************************
// Code Explanation
// **********************************
// Code Purpose:
// This code demonstrates controlling the ESP32-C6's built-in RGB LED to cycle through colors,
// showcasing structured coding practices and non-blocking execution.

// Requirement:
// 1. Utilize the built-in RGB LED for color display.
// 2. Periodically print the current LED color.
// 3. Display a steady red color and print "Error" on simulated sensor read error.
// 4. Cycle colors with a non-blocking approach.

// Hardware Connection:
// Built-in RGB LED on GPIO8.

// New Created Functions:
// - setupLED(): Initializes the built-in RGB LED.
// - updateLED(): Cycles through colors in a non-blocking manner.

// Security Considerations:
// - Monitor for consistent power supply to prevent malfunction.
// - Implement error handling for robust operation.

// Testing and Validation Approach:
// - Validate color cycling under various conditions.
// - Test error handling effectiveness.

// **********************************
// Libraries Import
// **********************************
#include <Arduino.h>           // Includes the main Arduino library for core functions.
#include "Adafruit_NeoPixel.h" // Includes the Adafruit NeoPixel library for controlling RGB LEDs.

// **********************************
// Constants and Struct Declaration
// **********************************
#define LED_PIN 8                                                  // Defines the GPIO pin number connected to the RGB LED.
#define LED_COUNT 1                                                // Defines the number of LEDs in the strip (just one for built-in RGB).
int pixelIndex = 0;                                                // Specifies the index of the pixel to control in the NeoPixel strip (only one LED here).
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); // Initializes the NeoPixel strip.

// Defines a struct for easy color management with predefined color values.
struct Colors
{
  static const uint32_t redColor = 0xFF0000;   // Red color value.
  static const uint32_t greenColor = 0x00FF00; // Green color value.
  static const uint32_t blueColor = 0x0000FF;  // Blue color value.
  static const uint32_t offColor = 0x000000;   // Represents the LED turned off.
};

// **********************************
// Class Declaration
// **********************************
// Defines a class for controlling the built-in RGB LED.
// The class implements a non-blocking color cycling approach.
// The class also implements error handling for sensor read failure.
// **********************************

// Declare a class to encapsulate LED control logic.
class LedController
{
public:
  // Constructor initializes the LED's current state, timing variables, and interval.
  LedController() : currentLedState(GREEN), previousMillis(0), interval(1000) {}

  // Method to correctly initialize timing for the first LED update.
  void setup()
  {
    previousMillis = millis();
  }

  // Method to update the LED's color based on the current state.
  void updateLed()
  {
    unsigned long currentMillis = millis(); // Capture the current time.
    // Check if the interval has elapsed to change the LED color.
    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis; // Update the timing for the next color change.
      // Switch statement to update the LED color based on the current state.
      switch (currentLedState)
      {
      case GREEN:
        strip.setPixelColor(pixelIndex, Colors::greenColor); // Set LED to green.
        Serial.println("Current LED color: Green");          // Log current color.
        currentLedState = BLUE;                              // Move to the next color state.
        break;
      case BLUE:
        strip.setPixelColor(pixelIndex, Colors::blueColor); // Set LED to blue.
        Serial.println("Current LED color: Blue");          // Log current color.
        currentLedState = RED;                              // Move to the next color state.
        break;
      case RED:
        strip.setPixelColor(pixelIndex, Colors::redColor); // Set LED to red.
        Serial.println("Current LED color: Red");          // Log current color.
        currentLedState = GREEN;                           // Cycle back to the first color state.
        break;
      case ERROR:
        strip.setPixelColor(pixelIndex, Colors::redColor); // Set LED to red to indicate an error.
        Serial.println("Error: LED read failure");         // Log an error message.
        break;
      }
      strip.show(); // Apply the color update to the LED.
    }
  }

  // Method to simulate an error condition and demonstrate error handling.
  void simulateError()
  {
    currentLedState = ERROR; // Set the state to ERROR.
    updateLed();             // Update the LED to indicate the error.
    currentLedState = GREEN; // Reset the state after showing the error.
  }

private:
  // Enum to represent different LED states for color cycling and error indication.
  enum LedState
  {
    GREEN,
    BLUE,
    RED,
    ERROR
  } currentLedState;
  unsigned long previousMillis; // Tracks the last update time.
  const long interval;          // Interval between color changes.
};

// Instantiate the LedController class to manage the LED.
LedController ledController;

// **********************************
// Setup Functions
// **********************************
void setup()
{
  Serial.begin(115200);  // Initializes serial communication at 115200 baud rate.
  strip.begin();         // Initializes the NeoPixel strip.
  strip.show();          // Clears the strip at startup (turns off all LEDs).
  ledController.setup(); // Initialize the ledController timing.
}

// **********************************
// Loop Functions
// **********************************
void loop()
{
  ledController.updateLed(); // Call the update method to cycle LED colors.

  // Below code simulates an error condition every 15 seconds as a demonstration.
  static unsigned long lastErrorTime = 0; // Static variable to track the last error simulation time.
  if (millis() - lastErrorTime > 15000)
  {                                // Check if 15 seconds have passed.
    ledController.simulateError(); // Simulate an error.
    lastErrorTime = millis();      // Reset the timer for the next error simulation.
  }
}
