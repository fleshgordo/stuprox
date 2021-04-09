/*
   Simple demo, should work with any driver board

   Connect STEP, DIR as indicated

   Copyright (C)2015-2017 Laurentiu Badea

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.
*/
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"
#include <Servo.h>

#include <timer.h>

auto timer = timer_create_default();

Servo myServo;
// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 100

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

// 2-wire basic config, microstepping is hardwired on the driver
//BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

//Uncomment line to use enable/disable functionality
BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

//MultiDriver controller(stepperL, stepperR);
SyncDriver controller(stepperL, stepperR);

const float full_rotation_R = MOTOR_STEPS * MICROSTEPS * 2.5;
const float full_rotation_L = MOTOR_STEPS * MICROSTEPS * 7;
const int step_size_L = 140;
const byte interruptPin = 2;
boolean one_shot = false;
unsigned long start_time;
unsigned long current_time;

void setup() {
  myServo.attach(9);
  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);
  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);

  stepperR.enable();
  stepperL.enable();

  Serial.begin(115200);
  Serial.println("Postplotter booting ... ");
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), button_press, FALLING);
}

float step_L = 0.125;
float inc_step_L = 0.05;

bool switchServoOff(void *msg) {
  const char *m = (const char *)msg;
  myServo.write(90);
  Serial.print("print_message: ");
  Serial.println(m);
  return true; // repeat? true
}

void button_press() {
  current_time = millis();
  if (one_shot == false) {
    Serial.println("Starting servo ... ");
    myServo.write(95);
    timer.in(1000, switchServoOff, (void *)"switch servo off");
    start_time = millis();
  }
  one_shot = true;
  if (current_time - start_time > 2000) {

    Serial.println("triggered more than a second ago ...");
    one_shot = false;
  }
}

void loop() {
  //stepperR.enable();
  //stepperL.enable();
  timer.tick(); // tick the timer


  controller.move(full_rotation_L * -step_L, full_rotation_R * -0.5);
  step_L += inc_step_L;
  controller.move(full_rotation_L * step_L, full_rotation_R * -0.5);

  if (step_L > 1) {
    stepperR.disable();
    stepperL.disable();
  }
  //stepperR.disable();
  //stepperL.disable();
  //Serial.println("Drawing finished. ");
  //delay(1000);
}
