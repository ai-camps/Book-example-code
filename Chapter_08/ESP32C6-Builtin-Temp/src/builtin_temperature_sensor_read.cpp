//#include "builtin_temperature_sensor_read.h"
#include "builtin_temperature_sensor.h"
#include <Arduino.h>

static temperature_sensor_handle_t tsens; // Handle for temperature sensor

void initTemperatureSensor() {
    temperature_sensor_config_t tsens_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80); // Adjust the range as needed
    esp_err_t result = temperature_sensor_install(&tsens_config, &tsens);

    if (result == ESP_OK) {
        result = temperature_sensor_enable(tsens);
        if (result != ESP_OK) {
            Serial.println("Failed to enable temperature sensor");
        }
    } else {
        Serial.println("Failed to initialize temperature sensor");
    }
}

float readTemperatureSensor() {
    float temperature = 0.0;
    esp_err_t result = temperature_sensor_get_celsius(tsens, &temperature);

    if (result != ESP_OK) {
        Serial.println("Failed to read temperature");
        return NAN; // Return 'Not a Number' if there is a failure
    }

    return temperature;
}
