#include <Arduino.h>

const int IR_SENSOR_PIN = 2;
const int LED_PIN = LED_BUILTIN;

unsigned long lastDetectionTime = 0;
unsigned long lastMessageTime = 0;

void setup() {
    pinMode(IR_SENSOR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    Serial.begin(9600);
    Serial.println("TSOP38238 sensor test started");
    Serial.println("Point a TV remote at the sensor and press a button.");
}

void loop() {
    // TSOP38238 is active-low:
    // LOW means a compatible IR burst is being detected.
    if (digitalRead(IR_SENSOR_PIN) == LOW) {
        lastDetectionTime = millis();

        // Limit messages so the serial monitor remains readable.
        if (millis() - lastMessageTime >= 200) {
            Serial.println("IR detected");
            lastMessageTime = millis();
        }
    }

    // Keep the LED illuminated briefly after detection.
    if (millis() - lastDetectionTime < 100) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}
