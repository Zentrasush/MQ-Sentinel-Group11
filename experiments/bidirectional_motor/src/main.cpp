#include <Arduino.h>

const int SPEED_PIN = 5;      // D5 → EN/IN1
const int DIRECTION_PIN = 4;  // D4 → PH/IN2

const int TEST_SPEED = 128;         // PWM range: 0–255 so 50% PWM
const unsigned long RUN_TIME = 2000;
const unsigned long STOP_TIME = 1500;


// Motor Functions
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
  Serial.println("Bidirectional motor validation test");
  Serial.println("5 direction cycles will be performed");

  delay(2000);
}


void loop()
{
  static int cycleNumber = 1;
  if (cycleNumber > 5)
  {
    stopMotor();
    Serial.println("Validation test completed.");
    while (true)
    {
      // Test finished
    }
  }

  Serial.print("Cycle ");
  Serial.print(cycleNumber);
  Serial.println(": Direction A");
  rotateDirectionA();
  delay(RUN_TIME);
  stopMotor();
  Serial.println("Motor stopped");
  delay(STOP_TIME);


  Serial.print("Cycle ");
  Serial.print(cycleNumber);
  Serial.println(": Direction B");
  rotateDirectionB();
  delay(RUN_TIME);
  stopMotor();
  Serial.println("Motor stopped");
  delay(STOP_TIME);

  cycleNumber++;
}