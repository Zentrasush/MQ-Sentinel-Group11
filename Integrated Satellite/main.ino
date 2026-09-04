#include <Arduino.h>

const int IR_PIN = 2;         // D2 -> TSOP38238 OUT (Left)
const int SPEED_PIN = 5;      // D5 -> DRV8874 EN/IN1
const int DIRECTION_PIN = 4;  // D4 -> DRV8874 EN/IN2
const int LASER_PIN = 10;     // D10 -> transistor base (via 1k resistor)

// Motor setting
const int SEARCH_SPEED = 100;


const unsigned long IR_HOLD_TIME = 250;


const unsigned long DWELL_TIME_MS = 2000;


unsigned long last_irDetection = 0;
bool irDetected = false;

enum SystemState
{
  SEARCHING,
  TARGET_DETECTED
};

SystemState currentState = SEARCHING;

// Motor functions
void startMotor()
{
  digitalWrite(DIRECTION_PIN, LOW);
  analogWrite(SPEED_PIN, SEARCH_SPEED);
}

void stopMotor()
{
  analogWrite(SPEED_PIN, 0);
}

// Laser functions
void fireLaser()
{
  digitalWrite(LASER_PIN, HIGH);
}

void stopLaser()
{
  digitalWrite(LASER_PIN, LOW);
}


bool isOnTargetDuringDwell()
{
  static unsigned long lastSeen = 0;
  if (digitalRead(IR_PIN) == LOW)
  {
    lastSeen = millis();
  }
  return (millis() - lastSeen <= IR_HOLD_TIME);
}


bool holdOnTarget()
{
  fireLaser();
  unsigned long start = millis();

  while (millis() - start < DWELL_TIME_MS)
  {
    if (!isOnTargetDuringDwell())
    {
      stopLaser();
      Serial.println("Lost target before dwell time completed");
      return false;
    }
    delay(10);
  }

  stopLaser();
  Serial.println("Target held for full dwell time - scored");
  return true;
}

void setup()
{
  Serial.begin(9600);

  pinMode(IR_PIN, INPUT);
  pinMode(SPEED_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);

  // Establish the fixed direction before starting the motor.
  digitalWrite(DIRECTION_PIN, LOW);
  stopMotor();
  digitalWrite(LASER_PIN, LOW); // laser off at startup

  Serial.println("MQ Sentinel Week 5 MVP started");
  Serial.println("No IR: searching");
  Serial.println("IR detected: stop and fire laser");

  currentState = SEARCHING;
  startMotor();
}

// Core Loop logic
void loop()
{
  unsigned long currentTime = millis();

  if (digitalRead(IR_PIN) == LOW)
  {
    last_irDetection = currentTime;
    irDetected = true;
  }

  bool irRecentlyDetected =
      irDetected &&
      (currentTime - last_irDetection <= IR_HOLD_TIME);

  if (irRecentlyDetected)
  {
    // Stop immediately when the target is detected.
    if (currentState != TARGET_DETECTED)
    {
      stopMotor();
      currentState = TARGET_DETECTED;

      Serial.println("IR DETECTED -> motor stopped");

      // Hold and fire the laser while the alignment lasts.
      bool scored = holdOnTarget();
      Serial.println(scored ? "Scored this pass" : "Missed - target lost mid-dwell");

      // Either way, go back to searching for the next target.
      irDetected = false;
      startMotor();
      currentState = SEARCHING;
      Serial.println("NO IR -> searching");
    }
  }
  else
  {
    // The IR signal has been absent for longer than the hold time.
    if (currentState != SEARCHING)
    {
      irDetected = false;
      startMotor();
      currentState = SEARCHING;

      Serial.println("NO IR -> searching");
    }
  }
}
