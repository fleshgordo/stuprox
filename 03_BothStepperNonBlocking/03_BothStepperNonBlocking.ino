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

const long TURN_STEPS = MOTOR_STEPS * MICROSTEPS;

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
}

void loop()
{
  static bool started = false;

  if (!started)
  {
    stepperX.startMove(TURN_STEPS);
    stepperY.startMove(TURN_STEPS);
    started = true;
  }

  unsigned waitTimeX = stepperX.nextAction();
  unsigned waitTimeY = stepperY.nextAction();

  if (waitTimeX == 0)
  {
    stepperX.startMove(-TURN_STEPS);
  }

  if (waitTimeY == 0)
  {
    stepperY.startMove(-TURN_STEPS);
  }
}
