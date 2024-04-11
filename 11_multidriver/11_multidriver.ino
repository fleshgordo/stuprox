/*
   Uses multidriver with three stepper motors to perform simple
   synchronous or asynchronous movements

   Adapted 2024 by Gordan Savicic
   based on stepperdriver example Copyright (C)2015-2017 Laurentiu Badea

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.
*/
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// All the wires needed for full functionality
#define DIR_X 5
#define STEP_X 2

#define DIR_Y 6
#define STEP_Y 3
// Driver in CNC shield Z
#define DIR_Z 7
#define STEP_Z 4

// Uncomment line to use enable/disable functionality
#define SLEEP 8

// 2-wire basic config, microstepping is hardwired on the driver
// BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

// Uncomment line to use enable/disable functionality
BasicStepperDriver stepperX(MOTOR_STEPS, DIR_X, STEP_X, SLEEP);
BasicStepperDriver stepperY(MOTOR_STEPS, DIR_Y, STEP_Y, SLEEP);
BasicStepperDriver stepperZ(MOTOR_STEPS, DIR_Z, STEP_Z, SLEEP);

//MultiDriver controller(stepperL, stepperR);
SyncDriver controller(stepperX, stepperY, stepperZ);

void setup() {
  stepperX.begin(100, MICROSTEPS);
  stepperY.begin(100, MICROSTEPS);
  stepperZ.begin(100, MICROSTEPS);
  // this is needed for enabling/disabling steppers
  stepperX.setEnableActiveState(LOW);
  stepperY.setEnableActiveState(LOW);
  stepperZ.setEnableActiveState(LOW);

  stepperX.setSpeedProfile(BasicStepperDriver::CONSTANT_SPEED);
  stepperY.setSpeedProfile(BasicStepperDriver::CONSTANT_SPEED);
  stepperZ.setSpeedProfile(BasicStepperDriver::CONSTANT_SPEED);
  stepperX.enable();
  stepperY.enable();
  stepperZ.enable();
  Serial.begin(115200);
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("starting");
  controller.rotate(3600, 7200, 1800);
  Serial.println("finished");
}

void loop() {
  
}
