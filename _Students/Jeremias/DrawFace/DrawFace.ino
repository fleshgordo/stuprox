#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 4

// Since microstepping is set externally, make sure this matches the selected mode
// Set the jumper to middle position when using MICROSTEPS 4, no jumper = MICROSTEPS 1
// 1=full step, 2=half step etc.
#define MICROSTEPS 16

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

SyncDriver controller(stepperL, stepperR);

void setup()
{
  Serial.begin(115200);
  stepperL.begin(RPM, MICROSTEPS);
  stepperR.begin(RPM, MICROSTEPS);
  stepperL.setEnableActiveState(LOW);
  stepperR.setEnableActiveState(LOW);
}

const int numChars = 512;
char receivedChars[numChars];
boolean newData = false;

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
  static int index = 0;
  char endMarker = '\n';
  char receivingChar;
  while (Serial.available() > 0 && newData == false)
  {
    receivingChar = Serial.read();
    if (receivingChar != endMarker)
    {
      receivedChars[index] = receivingChar;
      index++;
      if (index >= numChars)
      {
        index = numChars - 1;
      }
    }
    else
    {
      receivedChars[index] = '\0';
      index = 0;
      newData = true;
    }
  }
}
void handleData()
{

  stepperL.enable();
  stepperR.enable();

  static byte index = 0;
  char *token;
  char *rest = receivedChars;
  char *command;
  char *cmdString;
  int moveL;
  int moveR;

  while ((cmdString = strtok_r(rest, ",", &rest)))
  {
    while ((token = strtok_r(cmdString, "_", &cmdString)))
    {
      if (index == 0)
        moveL = atoi(token);
      if (index == 1)
        moveR = atoi(token);
      index++;
    }
    controller.move(moveL, moveR);

    index = 0;
  }
  Serial.print("XX");

  stepperL.disable();
  stepperR.disable();
}
