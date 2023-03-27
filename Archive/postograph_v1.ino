// Interfacing Arduino uno with LDR sensor
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"

#define MOTOR_STEPS 200
#define RPM 120

#define MICROSTEPS 4

#define DIR_L 5
#define STEP_L 2

#define DIR_R 6
#define STEP_R 3

#define SLEEP 8

BasicStepperDriver stepperL(MOTOR_STEPS, DIR_L, STEP_L, SLEEP);
BasicStepperDriver stepperR(MOTOR_STEPS, DIR_R, STEP_R, SLEEP);

 MultiDriver controller(stepperL, stepperR);
//SyncDriver controller(stepperL, stepperR);

const float full_rotation_R = MOTOR_STEPS * MICROSTEPS * 2.5;
const float full_rotation_L = MOTOR_STEPS * MICROSTEPS * 7;

const int step_size_L = 500;

const int ledPinR = 5;
const int ledPinL = 2;

const int ldrPinR = A0;
const int ldrPinL = A1;

void setup()
{
    stepperL.begin(RPM, MICROSTEPS);
    stepperR.begin(RPM, MICROSTEPS);

    stepperL.setEnableActiveState(LOW);
    stepperR.setEnableActiveState(LOW);

    stepperR.enable();
    stepperL.enable();

    Serial.begin(9600);

    pinMode(ledPinL, OUTPUT);
    pinMode(ledPinR, OUTPUT);

    pinMode(ldrPinR, INPUT);
    pinMode(ldrPinL, INPUT);
}

void loop()
{
    ctrlStepperR(100);
    ctrlStepperL(100);
}

void ctrlStepperR(int threshold)
{
    int ldrStatusR = analogRead(ldrPinR);

    if (ldrStatusR <= threshold)
    {
        digitalWrite(ledPinR, HIGH);
        controller.move(0, full_rotation_R);
    }
    else
    {
        digitalWrite(ledPinR, LOW);
        controller.move(0, 0);
    }

    Serial.print("LDR R: ");
    Serial.println(ldrStatusR);
}

void ctrlStepperL(int threshold)
{
    int ldrStatusL = analogRead(ldrPinL);

    if (ldrStatusL <= threshold)
    {
        digitalWrite(ledPinL, HIGH);
        controller.move(full_rotation_L, 0);
    }
    else
    {
        digitalWrite(ledPinL, LOW);
        controller.move(0, 0);
    }

    Serial.print("LDR L: ");
    Serial.println(ldrStatusL);
}
