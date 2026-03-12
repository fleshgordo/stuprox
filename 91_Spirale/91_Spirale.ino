/*
   Simple demo with DRV8255 driver on CNC shield

   Connect STEP, DIR as indicated!
   Driverslots X and Y on the CNC shield are used

   Adapted 2021/2022 by Gordan Savicic
   based on Copyright (C)2015-2017 Laurentiu Badea

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.

*/
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 30

// Since microstepping is set externally, make sure this matches the selected mode
// Set the jumper to middle position when using MICROSTEPS 4, no jumper = MICROSTEPS 1
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// Driver in CNC shield X
#define DIR_L 5
#define STEP_L 2

// Driver in CNC shield Y
#define DIR_R 6
#define STEP_R 3

// Define the pin for enable/disable functionality
#define SLEEP 8

// Initialize the driver(s)
BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

//SyncDriver controller(stepperL, stepperR);
MultiDriver controller(stepperL, stepperR);

const int full_rotation_R = MOTOR_STEPS * MICROSTEPS * .5;
const int full_rotation_L = MOTOR_STEPS * MICROSTEPS;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting postplotter ... Faster your seatbelts! ");
  char buffer[40];
  sprintf(buffer, "Full rotation L: %d, Full rotation R: %d", full_rotation_L, full_rotation_R);
  Serial.println(buffer);

  //stepperL.setSpeedProfile(BasicStepperDriver::CONSTANT_SPEED);
  //stepperR.setSpeedProfile(BasicStepperDriver::CONSTANT_SPEED);

  // Pass some config to the instances and begin
  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);
  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);

  stepperR.enable();
  stepperL.enable();

  for (int i = 0; i < 21; i++) {
    controller.move(full_rotation_L, full_rotation_R / 20);
  }
  stepperR.disable();
  stepperL.disable();
  Serial.println("Drawing finished. ");
}

void loop()
{}
