/*
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\
                                            by Tom Pawlofsky & Gordan Savicic

5-BAR LINKAGE DRAWING ROBOT
Receives angle commands from p5.js via serial and moves stepper motors
Command format: X<angle>,Y<angle> (e.g., X45,Y90)

ANTI-JITTER OPTIMIZATION:
- Set DEBUG_MODE to false to reduce serial traffic
- JavaScript only sends commands when angles change > 0.5 degrees
- This prevents constant 60fps command flooding that causes motor jitter

IMPORTANT: The incoming values are in DEGREES but are currently used as STEPS
If you need angle-to-step conversion, uncomment the ANGLE_TO_STEPS section below
*/

#include <Arduino.h>
#include "BasicStepperDriver.h"

// ============================================================================
// STEPPER MOTOR CONFIGURATION
// ============================================================================

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60 // Reduce to 30 if experiencing jitter

// Microstepping configuration
// MUST match the jumper settings on your A4988/DRV8825 driver!
// 1=full step, 2=half step, 4=quarter step, 8=eighth step, 16=sixteenth step
// For A4988 1/4 step: MS1=HIGH, MS2=HIGH, MS3=LOW
#define MICROSTEPS 4

// ============================================================================
// GEAR RATIO & ANGLE CONVERSION
// ============================================================================
// If using gears or pulleys, define ratio here
// Example: If motor shaft has 20 teeth and arm shaft has 60 teeth, ratio = 3.0
#define GEAR_RATIO 1.0 // Direct drive = 1.0

// Calculate steps per degree for precise conversion
// Using startMove() with explicit step calculation for maximum precision
const float STEPS_PER_DEGREE = (MOTOR_STEPS * MICROSTEPS * GEAR_RATIO) / 360.0;

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================
// Set to false to reduce serial output and prevent motor jitter
#define DEBUG_MODE false

// ============================================================================
// PIN CONFIGURATION
// ============================================================================

// All the wires needed for full functionality
// Left stepper on CNC shield Y slot
#define DIR_L 6
#define STEP_L 3

// Right stepper on CNC shield X slot
#define DIR_R 5
#define STEP_R 2

// Enable/Sleep pin for both drivers
#define SLEEP 8

// Uncomment line to use enable/disable functionality
BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;      // toggle new message
boolean toggleEN = false;     // true = motors currently enabled

const int STARTUP_HOME_X = -90;
const int STARTUP_HOME_Y = 0;

int currentPositionL = STARTUP_HOME_Y;
int currentPositionR = STARTUP_HOME_X;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Booting postplotter..."));
  Serial.println(F("5-Bar Linkage Robot Ready"));
  Serial.println(F("Press ? for help"));

  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);

  // Disable acceleration for constant linear speed
  stepperL.setSpeedProfile(stepperL.LINEAR_SPEED);
  stepperR.setSpeedProfile(stepperR.LINEAR_SPEED);

  // this is needed for enabling/disabling steppers
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);

  stepperR.disable();
  stepperL.disable();

  Serial.println(F("Steppers initialized: DISABLED"));
  Serial.println(F("Commands in DEGREES (startMove)"));
  Serial.print(F("Steps per degree: "));
  Serial.println(STEPS_PER_DEGREE);
  Serial.print(F("1 full rotation = "));
  Serial.print(MOTOR_STEPS * MICROSTEPS);
  Serial.println(F(" steps"));
  Serial.print(F("Startup sync position X"));
  Serial.print(STARTUP_HOME_X);
  Serial.print(F(",Y"));
  Serial.println(STARTUP_HOME_Y);

  Serial.println(F("Manually set motors to X-90,Y0, then enable from web UI"));
}

void loop()
{
  receiveData();
  if (newData == true)
  {
    handleData();
    newData = false;
  }
}

void receiveData()
{
  static byte ndx = 0;
  char endMarker = '\n';
  char receivingChar;
  while (Serial.available() > 0 && newData == false)
  {
    receivingChar = Serial.read();
    if (receivingChar != endMarker)
    {
      receivedChars[ndx] = receivingChar;
      ndx++;
      if (ndx >= numChars)
      {
        ndx = numChars - 1;
      }
    }
    else
    {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void handleData()
{
#if DEBUG_MODE
  Serial.print(F("DEBUG: This came in ... "));
  Serial.println(receivedChars);
#endif

  if (receivedChars[0] == 'X')
  {
    int pos_X;
    int pos_Y;
    sscanf(receivedChars, "X%d,Y%d", &pos_X, &pos_Y);

#if DEBUG_MODE
  Serial.print(F("*** *** Received X:"));
    Serial.print(pos_X);
  Serial.print(F(" Y:"));
    Serial.println(pos_Y);
#endif

    // Safety limits
    if (pos_X < -1000 || pos_Y < -1000 || pos_Y > 1000 || pos_X > 1000)
    {
      Serial.println(F("ERROR: Position out of range!"));
      pos_X = 0;
      pos_Y = 0;
    }

    // MOTOR MAPPING: L motor = Y command, R motor = X command
    // startRotate() expects DEGREES, so we pass angles directly
    int targetPositionL = pos_Y; // Left motor uses Y (in degrees)
    int targetPositionR = pos_X; // Right motor uses X (in degrees)

#if DEBUG_MODE
  Serial.print(F("*** Target angles - L:"));
    Serial.print(targetPositionL);
  Serial.print(F(" deg R:"));
    Serial.print(targetPositionR);
  Serial.println(F(" deg"));
#endif

    // Calculate degrees to move
    int degreesToMoveL = targetPositionL - currentPositionL;
    int degreesToMoveR = targetPositionR - currentPositionR;

    // Convert degrees to steps for precise control
    long stepsToMoveL = round(degreesToMoveL * STEPS_PER_DEGREE);
    long stepsToMoveR = round(degreesToMoveR * STEPS_PER_DEGREE);

#if DEBUG_MODE
  Serial.print(F("*** Steps to move - L:"));
    Serial.print(stepsToMoveL);
  Serial.print(F(" ("));
    Serial.print(degreesToMoveL);
  Serial.print(F(" deg) R:"));
    Serial.print(stepsToMoveR);
  Serial.print(F(" ("));
    Serial.print(degreesToMoveR);
  Serial.println(F(" deg)"));
#endif

    // Use startMove for precise step control
    stepperL.startMove(stepsToMoveL);
    stepperR.startMove(stepsToMoveR);

    unsigned wait_time_microsL = 1;
    unsigned wait_time_microsR = 1;

    while (wait_time_microsL > 0 || wait_time_microsR > 0)
    {
      wait_time_microsL = stepperL.nextAction();
      wait_time_microsR = stepperR.nextAction();
    }

    // update current position (in degrees)
    currentPositionL = targetPositionL;
    currentPositionR = targetPositionR;

    // Always send a completion token so the web UI can release arduinoBusy
    // even when DEBUG_MODE is false.
    Serial.println(F("DONE"));

#if DEBUG_MODE
  Serial.print(F("*** *** Current position L:"));
    Serial.print(currentPositionL);
  Serial.print(F(" R:"));
    Serial.println(currentPositionR);
  Serial.println(F("*** DONE "));
#endif
  }

  else if (receivedChars[0] == 'H')
  {
    currentPositionL = STARTUP_HOME_Y;
    currentPositionR = STARTUP_HOME_X;
    Serial.print(F("Synced current position to X"));
    Serial.print(currentPositionR);
    Serial.print(F(",Y"));
    Serial.println(currentPositionL);
  }

  else if (receivedChars[0] == 'E')
  {
    // Toggle motor enable/disable state
    if (toggleEN)
    {
      // Motors are currently enabled, so disable them
      stepperR.disable();
      stepperL.disable();
      Serial.println(F("Steppers DISABLED - coils disengaged"));
    }
    else
    {
      // Motors are currently disabled, so enable them
      stepperR.enable();
      stepperL.enable();
      Serial.println(F("Steppers ENABLED - coils energized"));
    }
    toggleEN = !toggleEN;
  }

  else if (receivedChars[0] == '?')
  {
    Serial.println(F("---------------- STUPX HELP ----------------"));
    Serial.println(F("Commands:"));
    Serial.println(F("  X<val>,Y<val>  move steppers in DEGREES"));
    Serial.println(F("  E              toggle enable/disable steppers"));
    Serial.println(F("  H              sync startup position X-90,Y0"));
    Serial.println(F("  ?              show this help menu"));
    Serial.println(F("Configuration:"));
    Serial.print(F("  Motor steps: "));
    Serial.println(MOTOR_STEPS);
    Serial.print(F("  Microstepping: 1/"));
    Serial.println(MICROSTEPS);
    Serial.print(F("  RPM: "));
    Serial.println(RPM);
    Serial.print(F("  Steps per revolution: "));
    Serial.println(MOTOR_STEPS * MICROSTEPS);
    Serial.print(F("  Steps per degree: "));
    Serial.println(STEPS_PER_DEGREE);
    Serial.println(F("  Mode: angle input with explicit step conversion"));
    Serial.println(F("Current Status:"));
    Serial.print(F("  Steppers: "));
    Serial.println(toggleEN ? F("ENABLED") : F("DISABLED"));
    Serial.print(F("  Position L: "));
    Serial.print(currentPositionL);
    Serial.print(F(" deg  Position R: "));
    Serial.print(currentPositionR);
    Serial.println(F(" deg"));
    Serial.println(F("--------------------------------------------"));
  }
}
