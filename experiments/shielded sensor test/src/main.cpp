#include <Arduino.h>

const int FRONT_SENSOR_PIN = 6;
const int MOTOR_SPEED_PIN = 5;
const int LASER_PIN = 12;

const unsigned long SAMPLE_TIME = 1000;

bool detectedThisSample = false;
unsigned long sampleStartTime = 0;
int sampleNumber = 1;

void setup()
{
    Serial.begin(9600);

    pinMode(FRONT_SENSOR_PIN, INPUT);
    pinMode(MOTOR_SPEED_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);

    // Safe test condition.
    analogWrite(MOTOR_SPEED_PIN, 0);
    digitalWrite(LASER_PIN, LOW);

    sampleStartTime = millis();

    Serial.println("Unshielded front-sensor test");
    Serial.println("One result will appear every second");
}

void loop()
{
    unsigned long currentTime = millis();

    // TSOP38238 is active-low.
    if (digitalRead(FRONT_SENSOR_PIN) == LOW)
    {
        detectedThisSample = true;
    }

    if (currentTime - sampleStartTime >= SAMPLE_TIME)
    {
        Serial.print("Sample ");
        Serial.print(sampleNumber);
        Serial.print(": ");

        if (detectedThisSample)
        {
            Serial.println("DETECTED");
        }
        else
        {
            Serial.println("NO DETECTION");
        }

        detectedThisSample = false;
        sampleStartTime = currentTime;
        sampleNumber++;
    }
}