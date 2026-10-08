#include <Arduino.h>

// Pin Configuration 
const int LASER_PIN = 12;      // D12 -> Transistor base (via 1k resistor)
const int MAIN_SENSOR = 6;     // D6 -> TSOP38238 OUT

// Settings
const unsigned long DWELL_TIME_MS = 3000;  // 3 seconds
const unsigned long IR_HOLD_TIME = 250;    // IR burst gap tolerance
const unsigned long TARGET_REMOVAL_TIME = 500;

// Variables
bool laserActive = false;
bool targetSeen = false;
bool waitingForRelease = false;
bool timingRemoval = false;

unsigned long laserStartTime = 0;
unsigned long lastSeenTime = 0;
unsigned long removalStartTime = 0;

// Laser Functions
void fireLaser() {
    digitalWrite(LASER_PIN, HIGH);
}

void stopLaser() {
    digitalWrite(LASER_PIN, LOW);
}

// Setup
void setup() {
    pinMode(LASER_PIN, OUTPUT);
    pinMode(MAIN_SENSOR, INPUT);

    stopLaser();

    Serial.begin(9600);
    Serial.println("Laser Control System Ready");
}

// Main Loop
void loop() {
    unsigned long now = millis();

    // Detect IR signal (Active LOW)
    if (digitalRead(MAIN_SENSOR) == LOW) {
        targetSeen = true;
        lastSeenTime = now;
    }

    bool isOnTarget =
        targetSeen &&
        (now - lastSeenTime <= IR_HOLD_TIME);

    // Start Laser when target is detected
    if (!laserActive && !waitingForRelease && isOnTarget) {
        fireLaser();
        laserActive = true;
        laserStartTime = now;

        Serial.println("Target detected - Laser ON");
    }

    // Monitor laser firing
    if (laserActive) {

        // Stop laser if target is lost
        if (!isOnTarget) {
            stopLaser();
            laserActive = false;
            waitingForRelease = true;
            timingRemoval = false;

            Serial.println("Target lost - Laser OFF");
        }

        // Stop laser after 3 seconds
        else if (now - laserStartTime >= DWELL_TIME_MS) {
            stopLaser();
            laserActive = false;
            waitingForRelease = true;
            timingRemoval = false;

            Serial.println("3 seconds completed - Laser OFF");
        }
    }

    // Wait for target removal
    if (waitingForRelease) {
        if (isOnTarget) {
            timingRemoval = false;
        }
        else if (!timingRemoval) {
            timingRemoval = true;
            removalStartTime = now;
        }
        else if (now - removalStartTime >= TARGET_REMOVAL_TIME) {
            waitingForRelease = false;
            timingRemoval = false;
            targetSeen = false;

            Serial.println("Ready for next target");
        }
    }
}
