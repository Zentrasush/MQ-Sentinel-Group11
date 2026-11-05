#include <Arduino.h>

// Change these values between trials, upload, then reset the suspended body
// to the same marked start position before each run.
const int MOTOR_SPEED_PIN = 5;       // D5 -> DRV8874 EN/IN1
const int MOTOR_DIRECTION_PIN = 4;   // D4 -> DRV8874 PH/IN2
const int TEST_PWM = 80;             // 0 to 255
const int TEST_DIRECTION = LOW;      // LOW or HIGH; keep consistent within a set
const unsigned long PULSE_MS = 500;  // Change this to test pulse duration

void stopMotor() {
  analogWrite(MOTOR_SPEED_PIN, 0);
}

void setup() {
  Serial.begin(9600);
  pinMode(MOTOR_SPEED_PIN, OUTPUT);
  pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
  stopMotor();
  Serial.println("Rotation pulse calibration");
  Serial.print("PWM="); Serial.print(TEST_PWM);
  Serial.print(", direction="); Serial.print(TEST_DIRECTION);
  Serial.print(", pulse_ms="); Serial.println(PULSE_MS);
  Serial.println("Starting in 3 seconds. Measure body angle and residual motion.");
  delay(3000);
  digitalWrite(MOTOR_DIRECTION_PIN, TEST_DIRECTION);
  analogWrite(MOTOR_SPEED_PIN, TEST_PWM);
  delay(PULSE_MS);
  stopMotor();
  Serial.println("Pulse ended. Record final body angle and overshoot.");
}

void loop() {
  // One pulse per reset so each trial can start from the same reference.
}
