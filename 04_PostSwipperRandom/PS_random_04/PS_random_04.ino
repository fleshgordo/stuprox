// this sketch uses an array of custom strct point
// array is filled with random values for x and y
// instead of version 03 - the movements are absolute
// (version 02 movments were relative)
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

// defines for graphics / motion
#define NUMPTS 8
// structs ------------------------
struct Point
{
  float x;
  float y;
};

// global variables-----------------------------
// setting for dialogs
int penMountTime = 3; // in milli seconds
// setting for graphics
Point pts[NUMPTS];
int indexPt;
Point ptPen; // the target of the current movement, the postion if movement is done

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
  // random Seed from unused Analog pin
  randomSeed(analogRead(1));
  // random point setting
  InitPt(pts,MOTOR_STEPS*MICROSTEPS / 2 * -1, MOTOR_STEPS*MICROSTEPS / 2);
  PrintPtArray(pts,"Points");
  indexPt = 0;
  ptPen = {0,0};
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
}

void loopHide(){}

void loop() {
  // SyncDriver controller work
  controller.nextAction(); // move your ...
  // check if there is still some work
  if (!controller.isRunning())
  {
    Serial.print(" moving to point index ");
    Serial.println(indexPt);
    Serial.println("Point is");
    PrintPoint(pts[indexPt]);
    MoveTo(pts[indexPt]);
    // for next index
    indexPt = (indexPt + 1) % NUMPTS;
  }
}

void MoveTo(Point ptTarget)
{
  Point stepsToMove = SubtractPoints(ptTarget,ptPen);
  controller.startMove(stepsToMove.x, stepsToMove.y);
  ptPen = ptTarget;
}
