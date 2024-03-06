### Local Application Logic

Let's design a local application logic as follows:

1. The ESP32-C3 periodically retrieves temperature and humidity data from the DHT11 sensor.
2. The retrieved data is checked against a pre-defined normal range.
3. If the data is within the normal range, LED D4 (IO12) is turned off.
4. If the data exceeds the normal range, LED D4 (IO12) starts to blink.
5. In case of data retrieval failure, the system makes three attempts to read the data.
6. If all three attempts fail, LED D5 (IO13) glows steadily in red, indicating a failure state.

### ChatGPT Prompts for Coding

---

*Hi, ChatGPT,*

***Role:** Act as a senior software developer with expertise in embedded development, specifically with IoT projects using the ESP32-C3, a DHT11 sensor, a Piezo Buzzer, and Platformio IDE.* 

***Task:** Your task is to mentor a high school student (like me) who has a basic understanding of Python and is new to C++. I need you to develop a comprehensive code snippet that fulfills the following objective, detailed requirements, implementation guidance, and output format.*

***Objective:** Develop an educational code snippet for the ESP32-C3 to:*

- *Connect a DHT11 to Pin 19 (IO02) to read temperature and humidity data.*
- *Connect a Piezo Buzzer to Pin 24 (IO11) to sound an alert beep.*

***Requirements:***

1. ***Visual Indication:*** 
    1. *Use the LED D4 (IO12) on the ESP32-C3 to visually indicate the range of temperature and humidity data.*
    2. *Use the LED D5 (IO13) on the ESP32-C3 to visually signal any system errors.*
2. ***Sensor Data Reading:** Regularly read the DHT11 sensor's temperature and humidity data, and print it out locally in both Celsius and Fahrenheit.*
3. ***LED Feedback for Sensor Error:** If no data is read from the DHT11 sensor (indicating a read error), LED D5 should display a steady red color, the Piezo Buzzer should beep at a lower pitch, and "Sensor Error!" should be printed locally.*
4. ***Normal Condition Indication:** If the temperature and humidity data fall between the minimum and maximum values, LED D4 turns off, and the Piezo Buzzer remains muted.*
5. ***Abnormal Condition Indication:** If the temperature and humidity data fall outside the normal range, the LED D4 blinks red, and the Piezo Buzzer beeps at a higher pitch.*

***Implementation Guidance:***

- *Implement C++ programming best practices in this code snippet.*
- *Reference the files already uploaded in the "Knowledge" section.*
- *Declare the variables for temperature, humidity data range, and LED blink interval.*
- *Use a non-blocking approach and create a class to initialize, set color, and blink the built-in LED independently.*
- *Establish a non-blocking approach and create a class to independently control the Piezo Buzzer alert sound.*
- *If necessary, include and import the appropriate libraries.*
- *Address sensor communication issues with appropriate error handling.*

***Output Format:***

- *Use the pre-uploaded file format.*
- *Provide clear line-by-line comments.*

***Platformio.ini File:***

- *Create a new file that incorporates the required library dependencies and environment settings.*
- *Ensure the following information is included.*

```
[env:esp32-c3-devkitc-02]
platform = espressif32
board = esp32-c3-devkitc-02
framework = arduino
monitor_filters = esp32_exception_decoder, colorize
monitor_speed = 115200
board_build.flash_mode = dio
build_flags = 
	-DARDUINO_USB_MODE=1
	-DARDUINO_USB_CDC_ON_BOOT=1
```
