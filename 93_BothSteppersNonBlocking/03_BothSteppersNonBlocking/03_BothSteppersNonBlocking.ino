/*
   Draws a flower petal with both motors for demo purposes
   Try difference between SyncDriver and MultiDriver

   Adapted 2023/2024 by Gordan Savicic
   based on stepperdriver example Copyright (C)2015-2017 Laurentiu Badea

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.
*/
#include <Arduino.h>
#include "BasicStepperDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// All the wires needed for full functionality
#define DIR_L 5
#define STEP_L 2

#define DIR_R 6
#define STEP_R 3
// Uncomment line to use enable/disable functionality
#define SLEEP 8

// Uncomment line to use enable/disable functionality
BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

const int iterations = 10;

void setup() {
  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);
  // this is needed for enabling/disabling steppers
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);

  stepperR.enable();
  stepperL.enable();

  Serial.begin(115200);
  Serial.println("");
  Serial.println(" ____ _____ _   _ ______  __");
  Serial.println("/ ___|_   _| | | |  _ \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) \\  /");
  Serial.println(" ___) || | | |_| |  __//  \\");
  Serial.println("|____/ |_|  \\___/|_|  /_/\\_\\");
  Serial.println("");
}

void loop() {
  for (int i = 0; i < iterations; i++) {
    stepperL.startMove(360);
    stepperR.startMove(-360);

    unsigned wait_time_microsL = 1;
    unsigned wait_time_microsR = 1;
    while (wait_time_microsL > 0 || wait_time_microsR > 0) {
      wait_time_microsL = stepperL.nextAction(); 
      wait_time_microsR = stepperR.nextAction();

      // within this while loop you can execute other arduino code
    }
    delay(20);
  }

  stepperR.disable();
  stepperL.disable();
  Serial.println("Drawing finished. ");
}
