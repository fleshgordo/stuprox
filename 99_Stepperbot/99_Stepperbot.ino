/*
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\
                                            by Tom Pawlofsky & Gordan Savicic

*/

#include <Arduino.h>
#include "BasicStepperDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// All the wires needed for full functionality
#define DIR_L 5
#define STEP_L 2

#define DIR_R 6
#define STEP_R 3
// Uncomment line to use enable/disable functionality
#define SLEEP 8

// Uncomment line to use enable/disable functionality
BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

const byte numChars = 32;
char receivedChars[numChars];  // an array to store the received data
boolean newData = false;       // toggle new message
boolean toggleEN = false;

int currentPositionL = 0;
int currentPositionR = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Booting postplotter ... Fasten your seatbelts! ");
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("");
  Serial.println("Press ? for help");

  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);

  // this is needed for enabling/disabling steppers
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);

  stepperR.enable();
  stepperL.enable();
}

void loop() {
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
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void handleData() {
  Serial.print("DEBUG: This came in ... ");
  Serial.println(receivedChars);

  if (receivedChars[0] == 'X') {
    int pos_X;
    int pos_Y;
    sscanf(receivedChars, "X%d,Y%d", &pos_X, &pos_Y);
    char buffer[40];
    Serial.print("*** *** Moving stepper angle X:");
    Serial.print(pos_X);
    Serial.print(" Y:");
    Serial.println(pos_Y);

    // Map incoming values ()
    //int targetPositionL = map(pos_X, 0, 180, 0, 200);
    //int targetPositionR = map(pos_Y, 0, 180, 0, 200);

    int stepsToMoveL = pos_X - currentPositionL;
    int stepsToMoveR = pos_Y - currentPositionR;

    stepperL.startRotate(stepsToMoveL);
    stepperR.startRotate(stepsToMoveR);

    unsigned wait_time_microsL = 1;
    unsigned wait_time_microsR = 1;

    while (wait_time_microsL > 0 || wait_time_microsR > 0) {
      wait_time_microsL = stepperL.nextAction();
      wait_time_microsR = stepperR.nextAction();
    }

    // update current position
    currentPositionL += stepsToMoveL;
    currentPositionR += stepsToMoveR;

    Serial.print("*** *** Current position L:");
    Serial.print(currentPositionL);
    Serial.print(" R:");
    Serial.println(currentPositionR);
    Serial.println("*** DONE ");
  }

  else if (receivedChars[0] == 'E') {
    if (toggleEN) {
      stepperR.enable();
      stepperL.enable();
    } else {
      stepperR.disable();
      stepperL.disable();
    }
    toggleEN = !toggleEN;

  } else if (receivedChars[0] == '?') {
    Serial.println("------------------------------------------------------------");
    Serial.println("HELP ");
    Serial.println("------------------------------------------------------------");
    Serial.println("Use following commands to interact with servobot: ");
    Serial.println("X10,Y40 - rotate servo 1 to angle 10, rotate servo 2 to angle 40");
    Serial.println("E - toggle to enable/disable steppers");
  }
}
