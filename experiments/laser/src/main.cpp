#include <Arduino.h>

const uint8_t LASER_PIN = 8;

void setup()
{
    Serial.begin(9600);

    pinMode(LASER_PIN, OUTPUT);

    // Ensure laser begins switched off.
    digitalWrite(LASER_PIN, LOW);

    Serial.println("Independent laser test started");
}

void loop()
{
    // Turn laser on.
    digitalWrite(LASER_PIN, HIGH);
    Serial.println("Laser ON");
    delay(2000);

    // Turn laser off.
    digitalWrite(LASER_PIN, LOW);
    Serial.println("Laser OFF");
    delay(2000);
}