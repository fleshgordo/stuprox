/*
   Triggers an external interrupt on button press to disabled all stepper motors (aka killswitch)

   - Connect a button/switch to arduino pin 2 (uses internal pull-up resistor)
   - No stepper driver should be installed in Slot X (it uses pin 2 and 5)
   - Use slot Y (pins 7,4) and Z (pins 6,3)
   - This example will move both motors until "kill" switch is pressed

   Copyright (C)2021-2022 Gordan Savicic

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.
*/
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "SyncDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// All the wires needed for full functionality
#define DIR_L 7
#define STEP_L 4

#define DIR_R 6
#define STEP_R 3

//Uncomment line to use enable/disable functionality
#define SLEEP 8

BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

SyncDriver controller(stepperL, stepperR);

const float full_rotation_R = MOTOR_STEPS * MICROSTEPS * 2.5;
const float full_rotation_L = MOTOR_STEPS * MICROSTEPS * 7;

const byte interruptPin = 2;
boolean one_shot = false;

void setup() {
  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);
  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);

  stepperR.enable();
  stepperL.enable();

  Serial.begin(115200);
  Serial.println("Booting postplotter ... Faster your seatbelts! ");
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("");

  // Connect switch between interruptPin and GND
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), my_trigger, FALLING);
}

void my_trigger() {
  Serial.println("KILL SWITCH!!!");
  stepperR.disable();
  stepperL.disable();
  one_shot = true;
}

void loop() {
  if (one_shot == false) {
    controller.move(full_rotation_L, full_rotation_R);
    Serial.println("Rotating both motors ...");
    delay(2000);
  }
}
