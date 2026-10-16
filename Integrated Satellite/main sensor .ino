
const int MAIN_SENSOR = 6;

void setup() {
  Serial.begin(9600);
  pinMode(MAIN_SENSOR, INPUT_PULLUP);

  Serial.println("Single IR Sensor Test");
}

void loop() {
  int sensorValue = digitalRead(MAIN_SENSOR);

  if (sensorValue == LOW) {
    Serial.println("IR SIGNAL DETECTED");
  } else {
    Serial.println("NO IR SIGNAL");
  }

  delay(100);
}
