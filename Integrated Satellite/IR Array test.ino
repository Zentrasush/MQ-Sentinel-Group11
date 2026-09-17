#include <Arduino.h>

const int IR_FRONT       = 6;  // 0 deg
const int IR_FRONT_RIGHT = 7;  // 60 deg
const int IR_BACK_RIGHT  = 8;  // 120 deg
const int IR_BACK        = 9;  // 180 deg
const int IR_BACK_LEFT   = 11; // 240 deg
const int IR_FRONT_LEFT  = 12; // 300 deg

void setup() {
  pinMode(IR_FRONT, INPUT);
  pinMode(IR_FRONT_RIGHT, INPUT);
  pinMode(IR_BACK_RIGHT, INPUT);
  pinMode(IR_BACK, INPUT);
  pinMode(IR_BACK_LEFT, INPUT);
  pinMode(IR_FRONT_LEFT, INPUT);

  Serial.begin(9600);
  Serial.println("6-sensor IR array test - point the transmitter at each sensor in turn");
}

void loop() {
  // Sensors are active LOW - reads HIGH normally, drops LOW when they see
  // a 38kHz beacon.
  bool front      = digitalRead(IR_FRONT)       == LOW;
  bool frontRight = digitalRead(IR_FRONT_RIGHT) == LOW;
  bool backRight  = digitalRead(IR_BACK_RIGHT)  == LOW;
  bool back       = digitalRead(IR_BACK)        == LOW;
  bool backLeft   = digitalRead(IR_BACK_LEFT)   == LOW;
  bool frontLeft  = digitalRead(IR_FRONT_LEFT)  == LOW;

  Serial.print("FRONT: ");
  Serial.print(front ? "DETECTED" : "-");
  Serial.print("  FRONT_R: ");
  Serial.print(frontRight ? "DETECTED" : "-");
  Serial.print("  BACK_R: ");
  Serial.print(backRight ? "DETECTED" : "-");
  Serial.print("  BACK: ");
  Serial.print(back ? "DETECTED" : "-");
  Serial.print("  BACK_L: ");
  Serial.print(backLeft ? "DETECTED" : "-");
  Serial.print("  FRONT_L: ");
  Serial.println(frontLeft ? "DETECTED" : "-");

  delay(200);
}
