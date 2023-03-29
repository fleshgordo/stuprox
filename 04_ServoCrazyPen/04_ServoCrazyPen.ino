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

    Stepper in Slot X (Pin 5 & 2)
*/


#include <Arduino.h>
#include "BasicStepperDriver.h"
#include <Servo.h>

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 10

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

Servo servo;

const byte servoPin = 12;  // connect to Spindle enable pin (SpinEn) on CNC shield.
int pos = 90;
float speed = 0.1;
float angle = 0;

const int servo_min_ms = 800;
const int servo_max_ms = 2100;

const int servo_min_pos = 65;
const int servo_max_pos = 135;
const int servo_center_pos = 95;
const int servo_range = servo_max_pos - servo_min_pos;

// Initialize the driver(s)
BasicStepperDriver stepper(MOTOR_STEPS, DIR_X, STEP_X, SLEEP);

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
  stepper.enable();

  servo.write(servo_center_pos);
}

void loop() {


  stepper.startMove(100);

  unsigned wait_time_micros = 1;
  while (wait_time_micros > 0) {
    wait_time_micros = stepper.nextAction();
    // this is mapping the remaining steps to a range where the servo arm operates
    pos = servo_center_pos + servo_range * .5 * sin(angle);
    delay(5);
    servo.write(pos);
    angle += speed;
    //Serial.println(int(pos));
  }
  //delay(random(1000));

}
