#include <Arduino.h>

// Pin Assignments
const int IR_PIN = 2;    // D2 -> TSOP38238 OUT (Left)
const int SPEED_PIN = 5;  // D5 -> DRV8874 EN/IN1
const int DIRECTION_PIN = 4;    // D4 -> DRV8874 EN/IN2


// Motor setting
const int SEARCH_SPEED = 100;

// Prevent restarting between the target's individual bursts
const unsigned long IR_HOLD_TIME = 250;

// Program state
unsigned long last_irDetection = 0;
bool irDetected = false;

enum SystemState
{
    SEARCHING,
    TARGET_DETECTED
};

SystemState currentState = SEARCHING;


// Motor functions
void startMotor()
{
    digitalWrite(DIRECTION_PIN, LOW);
    analogWrite(SPEED_PIN, SEARCH_SPEED);
}

void stopMotor()
{
    analogWrite(SPEED_PIN, 0);
}


void setup()
{
    Serial.begin(9600);

    pinMode(IR_PIN, INPUT);
    pinMode(SPEED_PIN, OUTPUT);
    pinMode(DIRECTION_PIN, OUTPUT);

    // Establish the fixed direction before starting the motor.
    digitalWrite(DIRECTION_PIN, LOW);
    stopMotor();

    Serial.println("MQ Sentinel Week 5 MVP started");
    Serial.println("No IR: searching");
    Serial.println("IR detected: stop");

    currentState = SEARCHING;
    startMotor();
}


// Core Loop logic
void loop()
{
    unsigned long currentTime = millis();

    if (digitalRead(IR_PIN) == LOW)
    {
        last_irDetection = currentTime;
        irDetected = true;
    }

    bool irRecentlyDetected =
        irDetected &&
        (currentTime - last_irDetection <= IR_HOLD_TIME);

    if (irRecentlyDetected)
    {
        // Stop immediately when the target is detected.
        if (currentState != TARGET_DETECTED)
        {
            stopMotor();
            currentState = TARGET_DETECTED;

            Serial.println("IR DETECTED -> motor stopped");
        }
    }
    else
    {
        // The IR signal has been absent for longer than the hold time.
        if (currentState != SEARCHING)
        {
            irDetected = false;
            startMotor();
            currentState = SEARCHING;

            Serial.println("NO IR -> searching");
        }
    }
}