#include <Arduino.h>

// Assumed wiring: D5 -> DRV8874 EN/IN1, D4 -> PH/IN2.
const int MOTOR_SPEED_PIN = 5;
const int MOTOR_DIRECTION_PIN = 4;
const int MOTOR_PWM = 80;                 // 0 to 255; raise only if needed
const unsigned long RUN_MS = 1500;
const unsigned long STOP_MS = 1500;
const int CYCLES = 5;

void stopMotor() {
  analogWrite(MOTOR_SPEED_PIN, 0);
}

void runDirection(int directionLevel) {
  digitalWrite(MOTOR_DIRECTION_PIN, directionLevel);
  analogWrite(MOTOR_SPEED_PIN, MOTOR_PWM);
}

void setup() {
  Serial.begin(9600);
  pinMode(MOTOR_SPEED_PIN, OUTPUT);
  pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
  stopMotor();
  Serial.println("Motor direction check begins in 3 seconds.");
  Serial.println("Keep the mechanism clear and observe body/flywheel separately.");
  delay(3000);
}

void loop() {
  static int cycle = 0;
  if (cycle >= CYCLES) {
    stopMotor();
    Serial.println("Done. Reset Nano to repeat.");
    while (true) { }
  }

  cycle++;
  Serial.print("Cycle "); Serial.print(cycle); Serial.println(" - PH LOW");
  runDirection(LOW);
  delay(RUN_MS);
  stopMotor();
  delay(STOP_MS);

  Serial.print("Cycle "); Serial.print(cycle); Serial.println(" - PH HIGH");
  runDirection(HIGH);
  delay(RUN_MS);
  stopMotor();
  delay(STOP_MS);
}
