/*
   Serial port communication for controlling steppers

   Copyright (C)2021-2022 Gordan Savicic

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

SyncDriver controller_sync(stepperL, stepperR);
MultiDriver controller_async(stepperL, stepperR);

boolean stepper_acceleration = false;

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false; // toggle new message

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting postplotter ... Faster your seatbelts! ");
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("");
  Serial.println("Press ? for help");

  // Pass some config to the instances and begin
  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);
  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);
}

void loop()
{
  receiveData();
  if (newData == true) {
    handleData();
    newData = false;
  }
}

void receiveData() {
  static byte ndx = 0;
  char endMarker = '\n';
  char receivingChar;
  while (Serial.available() > 0 && newData == false) {
    receivingChar = Serial.read();
    if (receivingChar != endMarker) {
      receivedChars[ndx] = receivingChar;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void handleData() {
  Serial.print("DEBUG: This came in ... ");
  Serial.println(receivedChars);
  if (receivedChars[0] == 'E') {
    Serial.println("*** Both steppers enable");
    stepperR.enable();
    stepperL.enable();
  }
  else if (receivedChars[0] == 'D') {
    Serial.println("*** Both steppers disabled ");
    stepperR.disable();
    stepperL.disable();
  }
  else if (receivedChars[0] == 'A') {
    stepper_acceleration = !stepper_acceleration;
    if (stepper_acceleration) {
      Serial.println("*** Enabling acceleration ");
      stepperR.setSpeedProfile(stepperL.LINEAR_SPEED, 2000, 1000);
      stepperL.setSpeedProfile(stepperR.LINEAR_SPEED, 2000, 1000);
    }
    else {
      Serial.println("*** Disabling acceleration ");
      stepperL.setSpeedProfile(stepperL.CONSTANT_SPEED);
      stepperR.setSpeedProfile(stepperL.CONSTANT_SPEED);
    }
  }
  else if (receivedChars[0] == 'M') {
    int x_pos;
    int y_pos;
    sscanf(receivedChars, "M X%d,Y%d", &x_pos, &y_pos);
    char buffer[40];
    Serial.print("*** *** Moving motors to: X:");
    Serial.print(x_pos);
    Serial.print(" Y:");
    Serial.println(y_pos);

    controller_async.move(x_pos, y_pos);
    Serial.println("*** DONE ");
  }
  else if (receivedChars[0] == 'G') {
    int x_posG;
    int y_posG;
    sscanf(receivedChars, "G X%d,Y%d", &x_posG, &y_posG);
    Serial.print("*** Moving motors in non-blocking mode: X:");
    Serial.print(x_posG);
    Serial.print(" Y:");
    Serial.println(y_posG);

    stepperL.startMove(x_posG);
    stepperR.startMove(y_posG);
    unsigned wait_time_microsL = 1;
    unsigned wait_time_microsR = 1;
    while (wait_time_microsL > 0 || wait_time_microsR > 0 ) {
      wait_time_microsL = stepperL.nextAction();
      wait_time_microsR = stepperR.nextAction();
    }
    Serial.println("*** DONE ");
  }
  else if (receivedChars[0] == 'S') {
    int x_pos;
    int y_pos;
    sscanf(receivedChars, "S X%d,Y%d", &x_pos, &y_pos);
    Serial.print("*** Moving motors synchronously to: X:");
    Serial.print(x_pos);
    Serial.print(" Y:");
    Serial.println(y_pos);

    controller_sync.move(x_pos, y_pos);
    Serial.println("*** DONE ");
  }
  else if (receivedChars[0] == '?') {
    Serial.println("------------------------------------------------------------");
    Serial.println("HELP ");
    Serial.println("------------------------------------------------------------");
    Serial.println("Use following commands to interact with postplotter: ");
    Serial.println("");
    Serial.println("(E)nable both steppers");
    Serial.println("(D)isable both steppers");
    Serial.println("(M) X10,Y10 - Move stepper X 10 steps, Y steps (asynchronous mode)");
    Serial.println("(S) X10,Y10 - Move stepper X 10 steps, Y steps (synchronous mode)");
    Serial.println("(G) X10,Y10 - Move stepper X 10 steps, Y steps (in non blocking mode)");
    Serial.println("(A)cceleration enable/disable");
  }
}
