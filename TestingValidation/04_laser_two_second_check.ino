#include <Arduino.h>

// D12 controls the laser transistor driver, not the laser diode directly.
// First verify using a dummy LED where possible. Use a secure beam stop if
// testing the laser; never aim at people, eyes, vehicles, or reflective surfaces.
const int LASER_CONTROL_PIN = 12;
const unsigned long ON_TIME_MS = 2000;

void setup() {
  Serial.begin(9600);
  pinMode(LASER_CONTROL_PIN, OUTPUT);
  digitalWrite(LASER_CONTROL_PIN, LOW);
  Serial.println("Laser timing check starts in 3 seconds.");
  Serial.println("Confirm beam path is safely contained before using the laser.");
  delay(3000);
  digitalWrite(LASER_CONTROL_PIN, HIGH);
  unsigned long startTime = millis();
  while (millis() - startTime < ON_TIME_MS) { }
  digitalWrite(LASER_CONTROL_PIN, LOW);
  Serial.println("Control output was HIGH for 2000 ms, then switched LOW.");
}

void loop() {
  digitalWrite(LASER_CONTROL_PIN, LOW);
}
