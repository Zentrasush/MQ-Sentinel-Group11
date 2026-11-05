MQ Sentinel validation sketches

Hardware pin assumptions
- Arduino Nano D4 -> DRV8874 PH/IN2
- Arduino Nano D5 -> DRV8874 EN/IN1 (PWM)
- Main TSOP38238 OUT -> D6; left -> D7; right -> D8
- Laser transistor control -> D12 (never connect a laser diode directly to an Arduino pin)

Sketches
01_sensor_reception: continuously watches each sensor and logs whether it went LOW at least once in each 1-second window. Keep target distance/height fixed, set one angle, collect ten rows, then reposition.
02_motor_direction_check: alternates PH LOW/HIGH at PWM 80 for 1.5 seconds each, five cycles. Confirm the assembly is clear and mounted safely.
03_rotation_pulse_calibration: one adjustable motor pulse per reset. Change TEST_PWM, TEST_DIRECTION, and PULSE_MS. Mark the body start and measure body rotation and residual movement after each pulse.
04_laser_two_second_check: asserts the laser driver output for 2 seconds once. Verify with a dummy LED first; use beam containment for a real laser.

The five-minute, three-target end-to-end validation is run with the team's current integrated firmware, not these diagnostic sketches. Log the outcome in the workbook. Do not claim a 2-second target hold unless the laser remains on the scored target for the full interval.
