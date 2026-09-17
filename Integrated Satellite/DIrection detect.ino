#include <Arduino.h>

// 6 sensors, evenly spaced 60 degrees apart around the ring.
// FRONT is aligned with the laser boresight.
const int IR_FRONT       = 6;  // 0 deg
const int IR_FRONT_RIGHT = 7;  // 60 deg
const int IR_BACK_RIGHT  = 8;  // 120 deg
const int IR_BACK        = 9;  // 180 deg
const int IR_BACK_LEFT   = 11; // 240 deg
const int IR_FRONT_LEFT  = 12; // 300 deg

// Named directions, in the same order as the pins above, so index i
// corresponds to angle i * 60.
enum Direction
{
  DIR_NONE = -1,
  DIR_FRONT = 0,
  DIR_FRONT_RIGHT,
  DIR_BACK_RIGHT,
  DIR_BACK,
  DIR_BACK_LEFT,
  DIR_FRONT_LEFT
};

const char *directionName(Direction d)
{
  switch (d)
  {
    case DIR_FRONT:       return "FRONT";
    case DIR_FRONT_RIGHT: return "FRONT_RIGHT";
    case DIR_BACK_RIGHT:  return "BACK_RIGHT";
    case DIR_BACK:        return "BACK";
    case DIR_BACK_LEFT:   return "BACK_LEFT";
    case DIR_FRONT_LEFT:  return "FRONT_LEFT";
    default:               return "NONE";
  }
}

void setup()
{
  pinMode(IR_FRONT, INPUT);
  pinMode(IR_FRONT_RIGHT, INPUT);
  pinMode(IR_BACK_RIGHT, INPUT);
  pinMode(IR_BACK, INPUT);
  pinMode(IR_BACK_LEFT, INPUT);
  pinMode(IR_FRONT_LEFT, INPUT);

  Serial.begin(9600);
  Serial.println("Target direction detection - 6 sensor array");
}

// Reads all 6 sensors and returns which one currently sees the beacon.
// If two ADJACENT sensors are both active at once, that usually means
// the target sits between them - this still reports the first (lower
// index) of the pair as a simple choice; upgrade this later to an
// interpolated angle if finer resolution is needed.
// Returns DIR_NONE if nothing is detected right now.
Direction detectDirection()
{
  bool state[6];
  state[DIR_FRONT]       = digitalRead(IR_FRONT)       == LOW;
  state[DIR_FRONT_RIGHT] = digitalRead(IR_FRONT_RIGHT) == LOW;
  state[DIR_BACK_RIGHT]  = digitalRead(IR_BACK_RIGHT)  == LOW;
  state[DIR_BACK]        = digitalRead(IR_BACK)        == LOW;
  state[DIR_BACK_LEFT]   = digitalRead(IR_BACK_LEFT)   == LOW;
  state[DIR_FRONT_LEFT]  = digitalRead(IR_FRONT_LEFT)  == LOW;

  for (int i = 0; i < 6; i++)
  {
    if (state[i]) return (Direction)i;
  }

  return DIR_NONE;
}

void loop()
{
  Direction target = detectDirection();

  if (target != DIR_NONE)
  {
    Serial.print("Target direction: ");
    Serial.print(directionName(target));
    Serial.print(" (");
    Serial.print(target * 60);
    Serial.println(" deg)");
  }
  else
  {
    Serial.println("No target detected");
  }

  delay(150);
}
