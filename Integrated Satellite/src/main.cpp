#include <Arduino.h>

const int SPEED_PIN = 5;      // D5 -> EN/IN1
const int DIRECTION_PIN = 4;  // D4 -> PH/IN2
const int IR_PIN = 2;         // D2 -> TSOP38238 OUT
const int LASER_PIN = 10;     // D10 -> transistor base (via 1k resistor)

const int TEST_SPEED = 128;              // PWM range: 0-255, 50% PWM
const unsigned long RUN_TIME = 2000;     // max time per direction, ms
const unsigned long STOP_TIME = 1500;    // pause between cycles, ms
const unsigned long IR_HOLD_TIME = 250;  // debounce for beacon bursts
const unsigned long DWELL_TIME_MS = 2000; // required hold time to score

// ---------- Motor functions ----------
void stopMotor()
{
  analogWrite(SPEED_PIN, 0);
}

void rotateDirectionA()
{
  digitalWrite(DIRECTION_PIN, LOW);
  analogWrite(SPEED_PIN, TEST_SPEED);
}

void rotateDirectionB()
{
  digitalWrite(DIRECTION_PIN, HIGH);
  analogWrite(SPEED_PIN, TEST_SPEED);
}

// ---------- Laser functions ----------
void fireLaser()
{
  digitalWrite(LASER_PIN, HIGH);
}

void stopLaser()
{
  digitalWrite(LASER_PIN, LOW);
}

// ---------- Sensor check ----------

bool irDetected()
{
  static unsigned long lastSeen = 0;
  if (digitalRead(IR_PIN) == LOW)
  {
    lastSeen = millis();
  }
  return (millis() - lastSeen <= IR_HOLD_TIME);
}

bool holdAndFireLaser()
{
  fireLaser();
  unsigned long start = millis();

  while (millis() - start < DWELL_TIME_MS)
  {
    if (!irDetected())
    {
      stopLaser();
      Serial.println("Lost target before dwell time completed");
      return false;
    }
  }

  stopLaser();
  Serial.println("Target held for full dwell time - scored");
  return true;
}

// ---------- Combined rotation + detect + fire ----------
bool runUntilDetectionOrTimeout(unsigned long maxRunTime)
{
  unsigned long start = millis();

  while (millis() - start < maxRunTime)
  {
    if (irDetected())
    {
      stopMotor();
      Serial.println("IR detected mid-rotation -> motor stopped, firing laser");
      holdAndFireLaser();
      return true;
    }
  }

  stopMotor();
  return false;
}

void setup()
{
  pinMode(SPEED_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(LASER_PIN, OUTPUT);

  stopMotor();
  stopLaser();

  Serial.begin(9600);
  Serial.println("Full validation test: motor + IR sensor + laser");
  Serial.println("5 direction cycles will be performed");
  Serial.println("IR sensor active - rotation stops and laser fires on detection");

  delay(2000);
}

void loop()
{
  static int cycleNumber = 1;
  if (cycleNumber > 5)
  {
    stopMotor();
    stopLaser();
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
  runUntilDetectionOrTimeout(RUN_TIME);
  Serial.println("Motor stopped");
  delay(STOP_TIME);

  Serial.print("Cycle ");
  Serial.print(cycleNumber);
  Serial.println(": Direction B");
  rotateDirectionB();
  runUntilDetectionOrTimeout(RUN_TIME);
  Serial.println("Motor stopped");
  delay(STOP_TIME);

  cycleNumber++;
}
