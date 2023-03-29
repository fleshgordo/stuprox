/*
   This sketch controls two stepper motors in non-blocking mode using the Accelstepper library
   More info on the library are here: https://www.airspayce.com/mikem/arduino/AccelStepper/

   Connect STEP, DIR as indicated!
   Driverslots X and Y on the CNC shield are used

   Written by Gordan Savicic 2023

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.

  Stepper in Slot X (Pin 5 & 2) and Y (Pin 6 & 3)
*/


#include <AccelStepper.h>

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// Set the jumper to middle position when using MICROSTEPS 4, no jumper = MICROSTEPS 1
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// Driver in CNC shield X
#define DIR_X 5
#define STEP_X 2

// Driver in CNC shield Y
#define DIR_Y 6
#define STEP_Y 3

// Driver in CNC shield Z
#define DIR_Z 7
#define STEP_Z 4

// Define the pin for enable/disable functionality
#define SLEEP 8

// Initialize the steppers and the pins the will use
AccelStepper stepper1(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_Y, DIR_Y);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting Plotter ... Fasten your seatbelts! ");
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("");

  // initialize speed, acceleration and enable pin
  stepper1.setMaxSpeed(200.0);
  stepper1.setAcceleration(100.0);
  stepper1.setEnablePin(SLEEP);
  stepper1.enableOutputs();

  stepper2.setMaxSpeed(300.0);
  stepper2.setAcceleration(100.0);
  stepper2.setEnablePin(SLEEP);
  stepper2.enableOutputs();

  // make them move
  stepper1.moveTo(24);
  stepper2.moveTo(100);
}

void loop() {
  // Change direction at the limits
  if (stepper1.distanceToGo() == 0)
    stepper1.moveTo(-stepper1.currentPosition());
  if (stepper2.distanceToGo() == 0)
    stepper2.moveTo(-stepper2.currentPosition());

  // run executes a motor step if it's due, implementing accelerations 
  // and decelerations to achieve the target position. 
  stepper1.run();
  stepper2.run();
}
