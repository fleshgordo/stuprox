/*
   Simple demo, should work with any driver board

   Connect STEP, DIR as indicated

   Copyright (C)2015-2017 Laurentiu Badea

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.
*/
#include <Arduino.h>
#include "BasicStepperDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 70

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// All the wires needed for full functionality
#define DIR_L 5
#define STEP_L 2

#define DIR_R 6
#define STEP_R 3
//Uncomment line to use enable/disable functionality
#define SLEEP 8

// 2-wire basic config, microstepping is hardwired on the driver
//BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

//Uncomment line to use enable/disable functionality
BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

void setup() {
  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);
  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);
}

void loop() {

  // energize coils - the motor will hold position
  stepperR.enable();

  /*
     Moving motor one full revolution using the degree notation
  */
  //stepper.rotate(360);

  /*
     Moving motor to original position using steps
  */
  //stepper.move(-MOTOR_STEPS*MICROSTEPS*7); // 200*7 = 1400 steps to rotate the entire disk
  stepperR.move(-MOTOR_STEPS * MICROSTEPS * 2.5); // 200*7 = 1400 steps to rotate the entire disk
  // pause and allow the motor to be moved by hand
  stepperR.disable();

  delay(2000);
}
