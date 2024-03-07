### Local Application Logic

1. The ESP32-C3 periodically retrieves temperature and humidity data from the DHT11 sensor.
2. The retrieved data is checked against a pre-defined normal range.
3. Scenario #1 - If the retrieved data falls within the normal range, LED D4 (IO12) is turned off, and the buzzer is muted.
4. Scenario #2 - If the retrieved data falls outside the normal range, LED D4 (IO12) starts blinking red and the buzzer begins to beep with a low-pitch tone.
5. Scenario #3 - If data retrieval fails, the LED D5 (IO13) will turn red and the buzzer will sound a continuous high-pitched tone, indicating a failure state. The system will then attempt to read the data three times. If all three attempts fail, the system will initiate a reboot.
