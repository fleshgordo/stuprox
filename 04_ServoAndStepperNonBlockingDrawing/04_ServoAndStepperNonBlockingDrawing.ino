/*
  Simple non-blocking drawing sketch.

  - Two steppers (X/Y) move through an array of absolute XY target coordinates.
  - A servo on pin 12 moves with a random sine-based motion in parallel.
  - All motion is non-blocking and runs continuously in loop().
*/

#include <Arduino.h>
#include "BasicStepperDriver.h"
#include <Servo.h>

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 60

// Since microstepping is set externally, make sure this matches the selected mode
// 1=full step, 2=half step etc.
#define MICROSTEPS 4

// Driver in CNC shield X
#define DIR_X 5
#define STEP_X 2

// Driver in CNC shield Y
#define DIR_Y 6
#define STEP_Y 3

// Define the pin for enable/disable functionality
#define SLEEP 8

const int SERVO_PIN = 12;
const int SERVO_MIN_POS = 65;
const int SERVO_MAX_POS = 125;
const int SERVO_CENTER = 95;
const float SERVO_SPEED = 0.01;
const unsigned long SERVO_UPDATE_MS = 5;

BasicStepperDriver stepperX(MOTOR_STEPS, DIR_X, STEP_X, SLEEP);
BasicStepperDriver stepperY(MOTOR_STEPS, DIR_Y, STEP_Y, SLEEP);
Servo servo;

struct Point
{
  long x;
  long y;
};

// Keep this small and easy to modify.
Point path[] = {
    {0, 0},
    {800, 0},
    {800, 800},
    {0, 800},
    {0, 0},
    {400, 400}};

const int PATH_LEN = sizeof(path) / sizeof(path[0]);

long currentX = 0;
long currentY = 0;
long targetX = 0;
long targetY = 0;

int pathIndex = 0;
bool moveActive = false;
unsigned waitTimeX = 0;
unsigned waitTimeY = 0;

float servoAngle = 0.0;
unsigned long lastServoUpdate = 0;

void updateRandomServo()
{
  unsigned long now = millis();
  if (now - lastServoUpdate < SERVO_UPDATE_MS)
  {
    return;
  }
  lastServoUpdate = now;

  if (random(2) == 0)
  {
    servoAngle += SERVO_SPEED;
  }
  else
  {
    servoAngle -= SERVO_SPEED;
  }

  int servoRange = SERVO_MAX_POS - SERVO_MIN_POS;
  int servoPos = SERVO_CENTER + int(servoRange * 0.5 * sin(servoAngle));
  servoPos = constrain(servoPos, SERVO_MIN_POS, SERVO_MAX_POS);
  servo.write(servoPos);
}

void startMoveToPoint(int index)
{
  targetX = path[index].x;
  targetY = path[index].y;

  long dx = targetX - currentX;
  long dy = targetY - currentY;

  stepperX.startMove(dx);
  stepperY.startMove(dy);

  waitTimeX = 1;
  waitTimeY = 1;
  moveActive = true;

  Serial.print("Move to point ");
  Serial.print(index);
  Serial.print(" -> X:");
  Serial.print(targetX);
  Serial.print(" Y:");
  Serial.println(targetY);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting simple XY stepper path...");

  servo.attach(SERVO_PIN);
  servo.write(SERVO_CENTER);
  randomSeed(analogRead(A0));

  stepperX.begin(RPM, MICROSTEPS);
  stepperY.begin(RPM, MICROSTEPS);

  stepperX.setEnableActiveState(LOW);
  stepperY.setEnableActiveState(LOW);

  stepperX.setSpeedProfile(BasicStepperDriver::LINEAR_SPEED, 600, 600);
  stepperY.setSpeedProfile(BasicStepperDriver::LINEAR_SPEED, 600, 600);

  stepperX.enable();
  stepperY.enable();
}

void loop()
{
  updateRandomServo();

  if (!moveActive)
  {
    startMoveToPoint(pathIndex);
    pathIndex = (pathIndex + 1) % PATH_LEN;
    return;
  }

  waitTimeX = stepperX.nextAction();
  waitTimeY = stepperY.nextAction();

  if (waitTimeX == 0 && waitTimeY == 0)
  {
    currentX = targetX;
    currentY = targetY;
    moveActive = false;
  }
}
