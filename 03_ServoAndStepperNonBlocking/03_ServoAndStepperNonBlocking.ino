/*
   This sketch controls a servo motor and a stepper motor blocking mode

   Connect STEP, DIR as indicated!
   Driverslots X and Y on the CNC shield are used

   Written by Gordan Savicic 2023

   This file may be redistributed under the terms of the MIT license.
   A copy of this license has been included with this distribution in the file LICENSE.

   Wiring:
    Power supply +5v -> Servo + pin (red cable)
    Power supply GND -> CNC Shield GND pin
    Servo Data pin (orange or yellow) -> SpinEn on CNC shield (D12)
    Servo GND -> GND on CNC Shield

    Stepper in Slot Z (Pin 7 & 4)
*/


#include <Arduino.h>
#include "BasicStepperDriver.h"
#include <Servo.h>

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

// Driver in CNC shield Y
#define DIR_Z 7
#define STEP_Z 4

// Define the pin for enable/disable functionality
#define SLEEP 8

Servo servo;

const byte servoPin = 12;  // connect to Spindle enable pin (SpinEn) on CNC shield.
int pos = 90;
float speed = .1;

const int servo_min_ms = 800;
const int servo_max_ms = 2100;

const int servo_min_pos = 55;
const int servo_max_pos = 135;
const int servo_range = servo_max_pos - servo_min_pos;

const float full_rotation_L = MOTOR_STEPS * MICROSTEPS * 14;
const int step_size = 360;

// Initialize the driver(s)
BasicStepperDriver stepper(MOTOR_STEPS, DIR_Z, STEP_Z, SLEEP);

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

  // Pass some config to the instances and begin
  stepper.begin(RPM, MICROSTEPS);

  // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
  stepper.setEnableActiveState(LOW);

  // attach the servo
  servo.attach(servoPin);
  // energize coils


  servo.write(pos);

  Serial.println(pos);


  Serial.println("Drawing finished. ");
}

int final_pos = MOTOR_STEPS * MICROSTEPS * 5;
bool toggle_pen = false;

void loop() {
  while (final_pos > 100) {
    stepper.enable();
    stepper.startMove(final_pos);

    final_pos -= 100;
    unsigned wait_time_micros = 1;
    while (wait_time_micros > 0) {
      wait_time_micros = stepper.nextAction();
      int remaining = stepper.getStepsRemaining();
      //int new_pos = servo_max_pos - (servo_max_pos / (MOTOR_STEPS * MICROSTEPS) * remaining);
      int new_pos = 0;
      if (toggle_pen) {
        new_pos = servo_max_pos - ((float(servo_range) / final_pos) * remaining);
      } else {
         new_pos = servo_min_pos - ((float(servo_range) / final_pos) * -1 *remaining) ;
      }
      servo.write(new_pos);
      //Serial.println(new_pos);
    }
    delay(500);
    toggle_pen = !toggle_pen;
    stepper.disable();
  }
}
