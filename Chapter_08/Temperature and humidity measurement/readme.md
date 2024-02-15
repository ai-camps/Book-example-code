Instruction Prompt to ChatGPT

Hi, ChatGPT, please follow my below instructions:

Objective: Write a code snippet for the ESP32-C6 to interface with a DHT11 temperature and humidity sensor. The program should periodically read data from the sensor and indicate the sensor's status and measured environmental conditions using ESP32-C6 builtin RGB LED.

Requirements:
1. Sensor Data Reading:The code snippet should read temperature (in both Celsius and Fahrenheit) and humidity data from the DHT11 sensor every 3 seconds.
2. LED Feedback for Sensor Error: If no data is read from the DHT11 sensor (indicating a read error), the ESP32-C6's RGB LED should blink red at a frequency of every 100ms.
3. Normal Condition Indication: If the temperature is within the range of 15°C to 25°C and the humidity is between 10%RH to 60%RH, the RGB LED should display a steady green color.
4. Low Temperature/Humidity Indication: If the temperature drops below 15°C or the humidity falls below 10%RH, change the RGB LED to a steady blue color.
5. High Temperature/Humidity Indication: If the temperature exceeds 25°C or the humidity exceeds 60%RH, change the RGB LED to a steady red color.

Implementation Notes:
- Refer to the "ESP32-C6 Specifics" for information about the ESP32-C built-in addressable RGB LED and temperature sensor. This information can be found in the pre-defined instructions of the GPT "ESP32-C6 Coding Assistant".
- Refer to the "ESP32-C6-Pinout.txt" file for PIN definitions from the knowledge of the GPT "ESP32-C6 Coding Assistant".
- Utilize appropriate libraries for interfacing with the DHT11 sensor.
- Implement functions for reading sensor data, converting temperature to Fahrenheit, and controlling the RGB LED color.
- Ensure that the program includes error handling for sensor communication issues.
- Provide comments within the code to explain the logic and any important steps or decisions.

Output Format:
- The code snippet output format must exactly follow the pre-defined instruction from the GPT of “ESP32-C6 Coding Assistant”
- The code snippet output shall print out locally to show current temperature data by Fahrenheit and Celsius, and humidity data.
