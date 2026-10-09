#include <Arduino.h>

// Pins
const int DIRECTION_PIN = 4;        // D4 -> PH/IN2
const int SPEED_PIN = 5;            // D5 -> EN/IN1
const int MAIN_SENSOR = 6;          // D6 -> TSOP38238 OUT
const int LEFT_SENSOR = 7;          // D7 -> TSOP38238 OUT
const int RIGHT_SENSOR = 8;         // D8 -> TSOP38238 OUT
const int LASER_PIN = 12;           // D12 -> transistor base (via 1k resistor)


// Settings
const int TURN_LEFT = LOW;
const int TURN_RIGHT = HIGH;
const int MOTOR_SPEED = 80;

// Experimental stopping pulse
const int STOPPING_SPEED = 60;
const unsigned long STOPPING_TIME = 100;
const unsigned long STOPPING_PAUSE_TIME = 200;
const bool STOPPING_OPPOSITE_DIRECTION = true;

const unsigned long SEARCH_TIME = 1500;
const unsigned long REVERSE_PAUSE_TIME = 200;
const unsigned long IR_HOLD_TIME = 250;
const unsigned long CONFIRM_TIME = 150;

// Samiha's laser control setting
const unsigned long DWELL_TIME_MS = 3000;

const unsigned long TARGET_REMOVAL_TIME = 500;


// Sensor indices: 0 = main, 1 = left, 2 = right
const int sensor[3] = {
    MAIN_SENSOR,
    LEFT_SENSOR,
    RIGHT_SENSOR
};

bool sensorSeen[3] = {false, false, false};
unsigned long lastSeen[3] = {0, 0, 0};
bool detected[3] = {false, false, false};


// System States
enum SystemState {
    SEARCHING,
    STOPPING,
    CONFIRMING,
    FIRING,
    WAITING_FOR_TARGET_REMOVAL
};

SystemState state = SEARCHING;


// Variables
int motorDirection = TURN_RIGHT;
int pendingDirection = TURN_RIGHT;

bool reversing = false;
bool timingRemoval = false;
bool stoppingPulseStarted = false;

int stoppingDirection = TURN_LEFT;

unsigned long stoppingStartTime = 0;
unsigned long stoppingPulseStartTime = 0;
unsigned long directionStartTime = 0;
unsigned long reverseStartTime = 0;
unsigned long confirmationStartTime = 0;
unsigned long laserStartTime = 0;
unsigned long removalStartTime = 0;


// Motor functions
void stopMotor() {
    analogWrite(SPEED_PIN, 0);
}

void startMotor() {
    digitalWrite(DIRECTION_PIN, motorDirection);
    analogWrite(SPEED_PIN, MOTOR_SPEED);
}


// Samiha's Laser Control Functions
void fireLaser() {
    digitalWrite(LASER_PIN, HIGH);
}

void stopLaser() {
    digitalWrite(LASER_PIN, LOW);
}


// Sensor signal check
// Bug 1 fixed: sensorIndex avoids variable collision
bool irDetected(int sensorIndex, unsigned long now) {
    if (digitalRead(sensor[sensorIndex]) == LOW) {
        sensorSeen[sensorIndex] = true;
        lastSeen[sensorIndex] = now;
    }

    return sensorSeen[sensorIndex] &&
           (now - lastSeen[sensorIndex] <= IR_HOLD_TIME);
}


void beginSearching(unsigned long now) {
    stopLaser();
    reversing = false;
    timingRemoval = false;
    directionStartTime = now;
    state = SEARCHING;

    Serial.println("SEARCHING for meteors");
}

void beginConfirming(unsigned long now) {
    stopMotor();
    confirmationStartTime = now;
    state = CONFIRMING;

    Serial.println("CONFIRMING: stopping stage finished");
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
            motorDirection == TURN_LEFT ? TURN_RIGHT : TURN_LEFT;
    }

    if (STOPPING_TIME == 0 || STOPPING_SPEED == 0) {
        beginConfirming(now);
    }
    else {
        state = STOPPING;
        Serial.println("STOPPING: brake pause, then calibrated pulse");
    }
}

void requestReverse(int direction, unsigned long now) {
    stopMotor();
    pendingDirection = direction;
    reversing = true;
    reverseStartTime = now;
}

void updateSearch(unsigned long now) {
    // Pausing before changing motor direction
    if (reversing) {
        if (now - reverseStartTime >= REVERSE_PAUSE_TIME) {
            motorDirection = pendingDirection;
            reversing = false;
            directionStartTime = now;
            startMotor();
        }
        return;
    }

    int wantedDirection = motorDirection;
    bool sideSensorGuidance = detected[1] != detected[2];

    if (sideSensorGuidance) {
        wantedDirection = detected[1] ? TURN_LEFT : TURN_RIGHT;
        directionStartTime = now;
    }
    else if (now - directionStartTime >= SEARCH_TIME) {
        wantedDirection =
            motorDirection == TURN_LEFT ? TURN_RIGHT : TURN_LEFT;
    }

    if (wantedDirection != motorDirection) {
        requestReverse(wantedDirection, now);
    }
    else {
        startMotor();
    }
}


// Setup
void setup() {
    Serial.begin(9600);

    for (int i = 0; i < 3; i++) {
        pinMode(sensor[i], INPUT);
    }

    pinMode(DIRECTION_PIN, OUTPUT);
    pinMode(SPEED_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);

    stopMotor();
    stopLaser();

    beginSearching(millis());
}


// MAIN LOOP
void loop() {
    unsigned long now = millis();

    for (int i = 0; i < 3; i++) {
        detected[i] = irDetected(i, now);
    }

    bool mainLOW = digitalRead(MAIN_SENSOR) == LOW;

    switch (state) {

        case SEARCHING:
            if (detected[0]) {
                beginStopping(now);
            }
            else {
                updateSearch(now);
            }
            break;

        case STOPPING:
            // Laser remains OFF during stopping
            stopLaser();

            if (!stoppingPulseStarted) {
                stopMotor();

                if (now - stoppingStartTime >= STOPPING_PAUSE_TIME) {
                    digitalWrite(DIRECTION_PIN, stoppingDirection);
                    analogWrite(SPEED_PIN, STOPPING_SPEED);

                    stoppingPulseStartTime = now;
                    stoppingPulseStarted = true;
                }
            }
            else if (now - stoppingPulseStartTime >= STOPPING_TIME) {
                beginConfirming(now);
            }
            break;

        case CONFIRMING:
            stopMotor();

            if (!detected[0]) {
                Serial.println("Confirmation failed: target lost");
                beginSearching(now);
            }

            // Bug 2 fixed: correct 150 ms confirmation
            else if (
                (now - confirmationStartTime >= CONFIRM_TIME)
                && mainLOW
            ) {
                // Samiha's laser implementation
                fireLaser();
                laserStartTime = now;
                state = FIRING;

                Serial.println("FIRING: Laser ON");
            }
            break;

        case FIRING:
            stopMotor();

            // Samiha's laser control:
            // Turn OFF if target is lost
            if (!detected[0]) {
                stopLaser();

                Serial.println(
                    "Lost target before dwell time completed"
                );

                beginSearching(now);
            }

            // Turn OFF after 3 seconds
            else if (now - laserStartTime >= DWELL_TIME_MS) {
                stopLaser();

                timingRemoval = false;
                state = WAITING_FOR_TARGET_REMOVAL;

                Serial.println(
                    "Target held for full dwell time - scored"
                );
            }
            break;

        case WAITING_FOR_TARGET_REMOVAL:
            stopMotor();

            if (detected[0]) {
                timingRemoval = false;
            }
            else if (!timingRemoval) {
                timingRemoval = true;
                removalStartTime = now;
            }
            else if (now - removalStartTime >= TARGET_REMOVAL_TIME) {
                beginSearching(now);
            }
            break;
    }
}
