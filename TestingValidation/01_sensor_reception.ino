#include <Arduino.h>

// MQ Sentinel sensor test: TSOP38238 outputs are active-low.
const int MAIN_SENSOR_PIN = 6;
const int LEFT_SENSOR_PIN = 7;
const int RIGHT_SENSOR_PIN = 8;
const unsigned long SAMPLE_INTERVAL_MS = 1000;

unsigned long lastSample = 0;
int trialNumber = 0;
bool mainSeen = false;
bool leftSeen = false;
bool rightSeen = false;

void setup() {
  Serial.begin(9600);
  pinMode(MAIN_SENSOR_PIN, INPUT);
  pinMode(LEFT_SENSOR_PIN, INPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);
  lastSample = millis();
  Serial.println("Sensor reception test");
  Serial.println("Place target at one fixed angle and distance.");
  Serial.println("Each row is one 1-second trial; 1 means detected.");
  Serial.println("Trial,Main,Left,Right");
}

void loop() {
  // Watch continuously through the full window so brief IR envelope pulses are less likely to be missed.
  if (digitalRead(MAIN_SENSOR_PIN) == LOW) mainSeen = true;
  if (digitalRead(LEFT_SENSOR_PIN) == LOW) leftSeen = true;
  if (digitalRead(RIGHT_SENSOR_PIN) == LOW) rightSeen = true;

  if (millis() - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample += SAMPLE_INTERVAL_MS;
    trialNumber++;
    Serial.print(trialNumber); Serial.print(',');
    Serial.print(mainSeen ? 1 : 0); Serial.print(',');
    Serial.print(leftSeen ? 1 : 0); Serial.print(',');
    Serial.println(rightSeen ? 1 : 0);
    mainSeen = false;
    leftSeen = false;
    rightSeen = false;
  }
}
