/*
  Simple non-blocking XY stepper example.

  - Uses CNC shield slots X and Y.
  - Both steppers turn counter-clockwise continuously.
  - No path array/state machine; loop stays non-blocking.
*/

#include <Arduino.h>
#include "BasicStepperDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// Driver in CNC shield X
#define DIR_X 5
#define STEP_X 2

// Driver in CNC shield Y
#define DIR_Y 6
#define STEP_Y 3

// Driver in CNC shield Z (kept for compatibility/reference)
#define DIR_Z 7
#define STEP_Z 4

// Define the pin for enable/disable functionality
#define SLEEP 8

BasicStepperDriver stepperX(MOTOR_STEPS, DIR_X, STEP_X, SLEEP);
BasicStepperDriver stepperY(MOTOR_STEPS, DIR_Y, STEP_Y, SLEEP);

const long TURN_STEPS_MIN = (MOTOR_STEPS * MICROSTEPS) / 2;
const long TURN_STEPS_MAX = (MOTOR_STEPS * MICROSTEPS) * 3;
long TURN_STEPS = MOTOR_STEPS * MICROSTEPS;
bool movingForward = true;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting non-blocking XY steppers...");

  stepperX.begin(RPM, MICROSTEPS);
  stepperY.begin(RPM, MICROSTEPS);

  stepperX.setEnableActiveState(LOW);
  stepperY.setEnableActiveState(LOW);

  stepperX.setSpeedProfile(BasicStepperDriver::LINEAR_SPEED, 600, 600);
  stepperY.setSpeedProfile(BasicStepperDriver::LINEAR_SPEED, 600, 600);

  stepperX.enable();
  stepperY.enable();

  randomSeed(analogRead(A0));

  // Start with a forward move on both axes.
  // From here on, loop() keeps both axes synchronized and non-blocking.
  stepperX.startMove(TURN_STEPS);
  stepperY.startMove(TURN_STEPS);
}

void loop()
{
  // Each call advances at most one timing slice for each motor.
  // This keeps loop() free for other tasks and avoids blocking delays.
  unsigned waitTimeX = stepperX.nextAction();
  unsigned waitTimeY = stepperY.nextAction();

  // Only when BOTH axes are finished do we queue the next move.
  // This keeps direction changes in sync: forward together, backward together.
  if (waitTimeX == 0 && waitTimeY == 0)
  {
    // Flip direction after both motors complete the current segment.
    movingForward = !movingForward;
    // Pick a new move length for the next segment.
    TURN_STEPS = random(TURN_STEPS_MIN, TURN_STEPS_MAX + 1);
    // Positive = forward, negative = backward.
    long nextMove = movingForward ? TURN_STEPS : -TURN_STEPS;

    // Queue the same direction change on both axes.
    stepperX.startMove(nextMove);
    stepperY.startMove(nextMove);

    // Print current direction for quick debugging in Serial Monitor.
    Serial.print("Direction: ");
    Serial.print(movingForward ? "FORWARD" : "BACKWARD");
    Serial.print(" | Steps: ");
    Serial.println(TURN_STEPS);
  }
}
