/*
   This sketch controls two stepper motors in such a way that their coils should
   play a melody 
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

// Define note frequencies
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523

// Define note durations
#define Q 4  // quarter note
#define H 2  // half note
#define E 1  // eighth note

// Initialize stepper motor objects
AccelStepper stepper1(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_Y, DIR_Y);

// Define melody notes
int melody[] = {
  NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
  NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4
};

// Define note durations
int noteDurations[] = {
  Q, Q, Q, Q, Q, Q, H,
  Q, Q, Q, Q, Q, Q, H
};

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
  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(500);
  stepper1.setEnablePin(8);
  stepper1.enableOutputs();
  
  stepper2.setMaxSpeed(1000);
  stepper2.setAcceleration(500);
  stepper2.setEnablePin(8);
  stepper2.enableOutputs();
}

void loop() {
  // Play melody on stepper motors
  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
    // Calculate duration of note
    int duration = 1000 / noteDurations[i];

    // Play note on stepper1
    stepper1.setSpeed(melody[i]);
    stepper1.runSpeed();
    delay(duration / 2);
    stepper1.setSpeed(0);
    stepper1.runSpeed();

    // Play note on stepper2
    stepper2.setSpeed(melody[i]);
    stepper2.runSpeed();
    delay(duration / 2);
    stepper2.setSpeed(0);
    stepper2.runSpeed();

    // Delay between notes
    delay(duration / 2);
    Serial.println(duration);
  }
}
