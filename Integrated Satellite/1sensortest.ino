
#include <Arduino.h>

// =======================================
// MQ SENTINEL - SINGLE IR SENSOR VERSION
// =======================================

// PINS
const int DIRECTION_PIN = 4;  // DRV8874 PH
const int SPEED_PIN = 5;      // DRV8874 EN
const int MAIN_SENSOR = 6;    // TSOP38238 OUT
const int LASER_PIN = 12;     // Laser transistor

// MOTOR SETTINGS
const int TURN_LEFT = LOW;
const int TURN_RIGHT = HIGH;

const int MOTOR_SPEED = 80;

// EXPERIMENTAL STOPPING PULSE
const int STOPPING_SPEED = 60;
const unsigned long STOPPING_TIME = 100;
const unsigned long STOPPING_PAUSE_TIME = 200;
const bool STOPPING_OPPOSITE_DIRECTION = true;

// SEARCH SETTINGS
const unsigned long SEARCH_TIME = 1500;
const unsigned long REVERSE_PAUSE_TIME = 200;

// IR SETTINGS
const unsigned long IR_HOLD_TIME = 250;
const unsigned long CONFIRM_TIME = 150;

// LASER SETTINGS
const unsigned long DWELL_TIME_MS = 3000;
const unsigned long TARGET_REMOVAL_TIME = 500;

// IR SENSOR VARIABLES
bool sensorSeen = false;
unsigned long lastSeen = 0;
bool detected = false;

// SYSTEM STATES
enum SystemState {
    SEARCHING,
    STOPPING,
    CONFIRMING,
    FIRING,
    WAITING_FOR_TARGET_REMOVAL
};

SystemState state = SEARCHING;

// MOTOR VARIABLES
int motorDirection = TURN_RIGHT;
int pendingDirection = TURN_RIGHT;
int stoppingDirection = TURN_LEFT;

bool reversing = false;
bool timingRemoval = false;
bool stoppingPulseStarted = false;

// TIMERS
unsigned long stoppingStartTime = 0;
unsigned long stoppingPulseStartTime = 0;
unsigned long directionStartTime = 0;
unsigned long reverseStartTime = 0;
unsigned long confirmationStartTime = 0;
unsigned long laserStartTime = 0;
unsigned long removalStartTime = 0;

// =======================================
// MOTOR FUNCTIONS
// =======================================

void stopMotor() {
    analogWrite(SPEED_PIN, 0);
}

void startMotor() {
    digitalWrite(DIRECTION_PIN, motorDirection);
    analogWrite(SPEED_PIN, MOTOR_SPEED);
}

// =======================================
// LASER FUNCTIONS
// =======================================

void fireLaser() {
    digitalWrite(LASER_PIN, HIGH);
}

void stopLaser() {
    digitalWrite(LASER_PIN, LOW);
}

// =======================================
// SINGLE IR SENSOR DETECTION
// =======================================

bool irDetected(unsigned long now) {

    if (digitalRead(MAIN_SENSOR) == LOW) {
        sensorSeen = true;
        lastSeen = now;
    }

    return sensorSeen &&
           (now - lastSeen <= IR_HOLD_TIME);
}

// =======================================
// STATE FUNCTIONS
// =======================================

void beginSearching(unsigned long now) {

    stopLaser();

    reversing = false;
    timingRemoval = false;

    directionStartTime = now;
    state = SEARCHING;

    Serial.println("SEARCHING: Single IR sensor");
}

void beginConfirming(unsigned long now) {

    stopMotor();

    confirmationStartTime = now;
    state = CONFIRMING;

    Serial.println("CONFIRMING: Motor stopped");
}

void beginStopping(unsigned long now) {

    stopMotor();
    stopLaser();

    reversing = false;
    stoppingPulseStarted = false;
    stoppingStartTime = now;

    stoppingDirection = motorDirection;

    if (STOPPING_OPPOSITE_DIRECTION) {
        stoppingDirection =
            (motorDirection == TURN_LEFT)
            ? TURN_RIGHT
            : TURN_LEFT;
    }

    if (STOPPING_TIME == 0 ||
        STOPPING_SPEED == 0) {

        beginConfirming(now);

    } else {

        state = STOPPING;

        Serial.println(
            "STOPPING: Pause then reverse pulse"
        );
    }
}

// =======================================
// MOTOR DIRECTION REVERSAL
// =======================================

void requestReverse(int direction,
                    unsigned long now) {

    stopMotor();

    pendingDirection = direction;
    reversing = true;
    reverseStartTime = now;

    Serial.println("Reversing search direction");
}

// =======================================
// SINGLE SENSOR SEARCH LOGIC
// =======================================

void updateSearch(unsigned long now) {

    // Wait before changing direction
    if (reversing) {

        if (now - reverseStartTime >=
            REVERSE_PAUSE_TIME) {

            motorDirection = pendingDirection;
            reversing = false;

            directionStartTime = now;

            startMotor();
        }

        return;
    }

    // No left/right sensor guidance.
    // Scan in one direction for SEARCH_TIME.
    if (now - directionStartTime >= SEARCH_TIME) {

        int newDirection =
            (motorDirection == TURN_LEFT)
            ? TURN_RIGHT
            : TURN_LEFT;

        requestReverse(newDirection, now);

    } else {

        startMotor();
    }
}

// =======================================
// SETUP
// =======================================

void setup() {

    Serial.begin(9600);

    pinMode(MAIN_SENSOR, INPUT_PULLUP);

    pinMode(DIRECTION_PIN, OUTPUT);
    pinMode(SPEED_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);

    stopMotor();
    stopLaser();

    Serial.println("======================");
    Serial.println("MQ SENTINEL");
    Serial.println("SINGLE IR SENSOR MODE");
    Serial.println("======================");

    beginSearching(millis());
}

// =======================================
// MAIN LOOP
// =======================================

void loop() {

    unsigned long now = millis();

    // Read only MAIN sensor
    detected = irDetected(now);

    bool mainLOW =
        (digitalRead(MAIN_SENSOR) == LOW);

    switch (state) {

        // -------------------------------
        // SEARCHING
        // -------------------------------
        case SEARCHING:

            if (detected) {

                Serial.println("IR TARGET DETECTED");

                beginStopping(now);

            } else {

                updateSearch(now);
            }

            break;

        // -------------------------------
        // STOPPING
        // -------------------------------
        case STOPPING:

            stopLaser();

            if (!stoppingPulseStarted) {

                stopMotor();

                if (now - stoppingStartTime >=
                    STOPPING_PAUSE_TIME) {

                    digitalWrite(
                        DIRECTION_PIN,
                        stoppingDirection
                    );

                    analogWrite(
                        SPEED_PIN,
                        STOPPING_SPEED
                    );

                    stoppingPulseStartTime = now;
                    stoppingPulseStarted = true;
                }

            } else if (
                now - stoppingPulseStartTime >=
                STOPPING_TIME
            ) {

                beginConfirming(now);
            }

            break;

        // -------------------------------
        // CONFIRMING
        // -------------------------------
        case CONFIRMING:

            stopMotor();

            if (!detected) {

                Serial.println(
                    "Confirmation failed: target lost"
                );

                beginSearching(now);

            } else if (
                (now - confirmationStartTime >=
                 CONFIRM_TIME)
                && mainLOW
            ) {

                fireLaser();

                laserStartTime = now;
                state = FIRING;

                Serial.println(
                    "FIRING: Laser ON"
                );
            }

            break;

        // -------------------------------
        // FIRING
        // -------------------------------
        case FIRING:

            stopMotor();

            if (!detected) {

                stopLaser();

                Serial.println(
                    "Target lost before dwell completed"
                );

                beginSearching(now);

            } else if (
                now - laserStartTime >=
                DWELL_TIME_MS
            ) {

                stopLaser();

                timingRemoval = false;

                state =
                    WAITING_FOR_TARGET_REMOVAL;

                Serial.println(
                    "Laser dwell completed"
                );
            }

            break;

        // -------------------------------
        // WAITING FOR TARGET REMOVAL
        // -------------------------------
        case WAITING_FOR_TARGET_REMOVAL:

            stopMotor();

            if (detected) {

                timingRemoval = false;

            } else if (!timingRemoval) {

                timingRemoval = true;
                removalStartTime = now;

            } else if (
                now - removalStartTime >=
                TARGET_REMOVAL_TIME
            ) {

                beginSearching(now);
            }

            break;
    }
}
