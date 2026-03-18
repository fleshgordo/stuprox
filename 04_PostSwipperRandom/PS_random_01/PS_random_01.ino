// includes
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"

// defines - stepper
#define MOTOR_STEPS 200 // steps per revolution
#define RPM 60 //rounds per minute

// jumper for Microsteps
#define MICROSTEPS 4

// wiring via Cnc-Shield X and Y
#define DIR_X 5
#define STEP_X 2

#define DIR_Y 6
#define STEP_Y 3

#define SLEEP 8

// global variables-----------------------------
// setting for dialogs
int penMountTime = 3; // in milli seconds

// instances initalisation ----------------------------

// the stepper motors
BasicStepperDriver stepperX(MOTOR_STEPS, DIR_X, STEP_X, SLEEP);
BasicStepperDriver stepperY(MOTOR_STEPS, DIR_Y, STEP_Y, SLEEP);

//MultiDriver controller(stepperL, stepperR);
SyncDriver controller(stepperX, stepperY);


void setup() {
  // put your setup code here, to run once:
  // Initialize serial communication at 9600 baud rate
  Serial.begin(9600); // frequency of Serial-communication
  // set up steppers
  stepperX.begin(RPM, MICROSTEPS);
  stepperY.begin(RPM, MICROSTEPS);
  // this is needed for enabling/disabling steppers
  stepperX.setEnableActiveState(LOW);
  stepperY.setEnableActiveState(LOW);
  stepperX.setSpeedProfile(BasicStepperDriver::CONSTANT_SPEED);
  stepperY.setSpeedProfile(BasicStepperDriver::CONSTANT_SPEED);
  // let s start...
  Serial.println("hello Motors");
  Serial.print(penMountTime);
  Serial.println( " seconds to mount the pen");
  delay(penMountTime * 1000);
  // turn on Motors - above steppers are freely moveable
  stepperX.enable();
  stepperY.enable();
  // one turn for stepperX
  // half a turn for stepperY
  controller.startMove(MOTOR_STEPS*MICROSTEPS, MOTOR_STEPS*MICROSTEPS / 2);
}

void loop() {
  // SyncDriver controller work
  controller.nextAction(); // move your ...
  // check if there is still some work
  if (!controller.isRunning())
  {
    // random between one turn to left or right
    int stepsX = random(-1 * MOTOR_STEPS*MICROSTEPS, MOTOR_STEPS*MICROSTEPS);
    int stepsY = random(-1 * MOTOR_STEPS*MICROSTEPS, MOTOR_STEPS*MICROSTEPS);
    // tell the controller to do a movement
    // this is relative !! it is not the position, its added to the current position
    controller.startMove(stepsX,stepsY);
    Serial.println("new Movement");
    Serial.print("x: ");
    Serial.println(stepsX);
    Serial.print("y: ");
    Serial.println(stepsY);
  }

}
