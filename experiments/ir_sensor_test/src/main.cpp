#include <Arduino.h>

const int IR_SENSOR_PIN = 2;

void setup()
{
    Serial.begin(9600);

    // TSOP38238 normally outputs HIGH and produces LOW pulses when it receives a valid 38 kHz IR signal.
    pinMode(IR_SENSOR_PIN, INPUT);

    Serial.println("TSOP38238 IR sensor test started");
    Serial.println("Press a button on the remote...");
}

void loop()
{
    int sensorState = digitalRead(IR_SENSOR_PIN);

    if (sensorState == LOW)
    {
        Serial.println("IR signal detected!");

        // Prevent the Serial Monitor from being flooded.
        delay(100);
    }
}