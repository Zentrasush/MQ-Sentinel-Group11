
#include <Arduino.h>

// Encoder connections
const byte ENCODER_A = 2;
const byte ENCODER_B = 3;

// Encoder count
volatile long encoderCount = 0;

// Interrupt function
void encoderISR() {
  if (digitalRead(ENCODER_B) == LOW) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // Keep motor and laser disabled
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
  analogWrite(5, 0);
  digitalWrite(12, LOW);

  attachInterrupt(
    digitalPinToInterrupt(ENCODER_A),
    encoderISR,
    RISING
  );

  Serial.println("ENCODER TEST STARTED");
}

void loop() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 200) {
    lastPrint = millis();

    noInterrupts();
    long count = encoderCount;
    interrupts();

    Serial.print("Encoder Count: ");
    Serial.println(count);
  }
}
