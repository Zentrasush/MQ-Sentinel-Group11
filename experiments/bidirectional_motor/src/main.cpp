#include <Arduino.h>

const int SPEED_PIN = 5;      // D5 → EN/IN1
const int DIRECTION_PIN = 4;  // D4 → PH/IN2

const int TEST_SPEED = 128;         // PWM range: 0–255

void stopMotor() {
  analogWrite(SPEED_PIN, 0);
}

void rotateDirectionA() {
  digitalWrite(DIRECTION_PIN, LOW);
  analogWrite(SPEED_PIN, TEST_SPEED);
}

void rotateDirectionB() {
  digitalWrite(DIRECTION_PIN, HIGH);
  analogWrite(SPEED_PIN, TEST_SPEED);
}

void setup() {
  pinMode(SPEED_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);

  stopMotor();

  Serial.begin(9600);
  Serial.println("Bidirectional motor test starts in 3 seconds");

  delay(3000);
}

void loop() {
  Serial.println("Direction A");
  rotateDirectionA();
  delay(2000);

  Serial.println("Stopped");
  stopMotor();
  delay(2000);

  Serial.println("Direction B");
  rotateDirectionB();
  delay(2000);

  Serial.println("Stopped");
  stopMotor();
  delay(3000);
}