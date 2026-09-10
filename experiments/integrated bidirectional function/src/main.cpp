#include <Arduino.h>

// ==================================================
// Pin assignments
// ==================================================

// D2 and D3 are reserved for the motor encoder.
const int ENCODER_A_PIN = 2;
const int ENCODER_B_PIN = 3;

const int MOTOR_DIRECTION_PIN = 4;  // D4 -> DRV8874 PH/IN2
const int MOTOR_SPEED_PIN = 5;      // D5 -> DRV8874 EN/IN1

const int FRONT_SENSOR_PIN = 6;     // D6 -> TSOP38238 OUT

// D7-D11 are reserved for five additional IR sensors.

const int LASER_PIN = 12;           // D12 -> 1k resistor -> transistor base

// ==================================================
// Settings
// ==================================================

const int MOTOR_SPEED = 100;

// Motor runs in each direction for this long.
const unsigned long SEARCH_TIME = 1500;

// Motor stops briefly before reversing.
const unsigned long DIRECTION_PAUSE_TIME = 200;

// Continue treating the target as detected after an IR pulse.
const unsigned long IR_HOLD_TIME = 250;

// Target must remain detected before the laser fires.
const unsigned long CONFIRM_TIME = 150;

// Laser firing duration.
const unsigned long LASER_TIME = 2000;

// ==================================================
// System states
// ==================================================

enum SystemState
{
    SEARCHING,
    CONFIRMING,
    FIRING,
    WAITING_FOR_TARGET_REMOVAL
};

SystemState state = SEARCHING;

// ==================================================
// Variables
// ==================================================

int motorDirection = LOW;

bool motorPausedForReverse = false;
bool irSeen = false;

unsigned long directionStartTime = 0;
unsigned long pauseStartTime = 0;
unsigned long lastIrTime = 0;
unsigned long confirmationStartTime = 0;
unsigned long laserStartTime = 0;

// ==================================================
// Motor functions
// ==================================================

void startMotor()
{
    digitalWrite(MOTOR_DIRECTION_PIN, motorDirection);
    analogWrite(MOTOR_SPEED_PIN, MOTOR_SPEED);
}

void stopMotor()
{
    analogWrite(MOTOR_SPEED_PIN, 0);
}

void reverseMotor()
{
    if (motorDirection == LOW)
    {
        motorDirection = HIGH;
    }
    else
    {
        motorDirection = LOW;
    }

    startMotor();

    Serial.print("Motor direction: ");

    if (motorDirection == LOW)
    {
        Serial.println("A");
    }
    else
    {
        Serial.println("B");
    }
}

// ==================================================
// Laser functions
// ==================================================

void laserOn()
{
    digitalWrite(LASER_PIN, HIGH);
}

void laserOff()
{
    digitalWrite(LASER_PIN, LOW);
}

// ==================================================
// Search functions
// ==================================================

void beginSearching(unsigned long currentTime)
{
    laserOff();

    irSeen = false;
    motorPausedForReverse = false;

    startMotor();

    directionStartTime = currentTime;
    state = SEARCHING;

    Serial.println("SEARCHING: motor on, laser off");
}

void updateSearchMovement(unsigned long currentTime)
{
    // Motor is currently stopped before reversing.
    if (motorPausedForReverse)
    {
        if (currentTime - pauseStartTime >= DIRECTION_PAUSE_TIME)
        {
            reverseMotor();

            directionStartTime = currentTime;
            motorPausedForReverse = false;
        }
    }
    // Motor has completed its current search movement.
    else if (currentTime - directionStartTime >= SEARCH_TIME)
    {
        stopMotor();

        pauseStartTime = currentTime;
        motorPausedForReverse = true;

        Serial.println("Motor paused before reversing");
    }
}

// ==================================================
// Setup
// ==================================================

void setup()
{
    Serial.begin(9600);

    pinMode(FRONT_SENSOR_PIN, INPUT);

    pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
    pinMode(MOTOR_SPEED_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);

    stopMotor();
    laserOff();

    Serial.println("MQ Sentinel V2 started");

    beginSearching(millis());
}

// ==================================================
// Main loop
// ==================================================

void loop()
{
    unsigned long currentTime = millis();

    // TSOP38238 is active-low.
    if (digitalRead(FRONT_SENSOR_PIN) == LOW)
    {
        lastIrTime = currentTime;
        irSeen = true;
    }

    bool targetDetected =
        irSeen &&
        (currentTime - lastIrTime <= IR_HOLD_TIME);

    switch (state)
    {
        case SEARCHING:
        {
            if (targetDetected)
            {
                stopMotor();
                motorPausedForReverse = false;

                confirmationStartTime = currentTime;
                state = CONFIRMING;

                Serial.println("Target detected: confirming");
            }
            else
            {
                updateSearchMovement(currentTime);
            }

            break;
        }

        case CONFIRMING:
        {
            if (!targetDetected)
            {
                Serial.println("Target lost during confirmation");
                beginSearching(currentTime);
            }
            else if (currentTime - confirmationStartTime >= CONFIRM_TIME)
            {
                laserOn();

                laserStartTime = currentTime;
                state = FIRING;

                Serial.println("Target confirmed: laser on");
            }

            break;
        }

        case FIRING:
        {
            if (!targetDetected)
            {
                laserOff();

                Serial.println("Target lost: laser cancelled");
                beginSearching(currentTime);
            }
            else if (currentTime - laserStartTime >= LASER_TIME)
            {
                laserOff();

                state = WAITING_FOR_TARGET_REMOVAL;

                Serial.println("Laser completed two-second firing");
                Serial.println("Waiting for target removal");
            }

            break;
        }

        case WAITING_FOR_TARGET_REMOVAL:
        {
            // Prevent repeated firing at the same target.
            if (!targetDetected)
            {
                Serial.println("Target removed: restarting search");
                beginSearching(currentTime);
            }

            break;
        }
    }
}