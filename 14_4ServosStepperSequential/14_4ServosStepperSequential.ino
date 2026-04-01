/*
  14_4ServosStepperSequential.ino

  Four servos move one after another:
  Servo 1: startPos -> maxPos -> startPos
  Servo 2: startPos -> maxPos -> startPos
  Servo 3: startPos -> maxPos -> startPos
  Servo 4: startPos -> maxPos -> startPos

  At the same time, one stepper runs continuously.
  Change stepperSpeedStepsPerSec to adjust stepper speed.
*/

#include <Arduino.h>
#include <Servo.h>
#include <AccelStepper.h>

// -------------------- Servo configuration --------------------
const uint8_t SERVO_COUNT = 4;
const uint8_t servoPins[SERVO_COUNT] = {9, 10, 11, 12};
Servo servos[SERVO_COUNT];

const int startPos = 0; // Set this to any value >= 0
const int maxPos = 90;
const int servoStep = 1; // Degrees per step (bigger value = chunkier movement)
const unsigned long servoUpdateIntervalMs = 120; // Time between steps (higher = slower)

// -------------------- Stepper configuration --------------------
// CNC shield X slot by default: STEP=2, DIR=5, EN=8 (active LOW)
const uint8_t STEP_PIN = 2;
const uint8_t DIR_PIN = 5;
const uint8_t ENABLE_PIN = 8;

float stepperSpeedStepsPerSec = 400.0f; // Change this value to tune speed
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// -------------------- Sequence state --------------------
uint8_t activeServo = 0;
int currentPos = startPos;
int direction = 1; // +1 moving toward maxPos, -1 moving back to startPos
unsigned long lastServoUpdateMs = 0;
unsigned long lastStatusPrintMs = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("Boot: 4-servo sequential + continuous stepper");

    for (uint8_t i = 0; i < SERVO_COUNT; i++)
    {
        servos[i].attach(servoPins[i]);
        servos[i].write(startPos);
        Serial.print("Servo ");
        Serial.print(i + 1);
        Serial.print(" attached on pin ");
        Serial.print(servoPins[i]);
        Serial.print(", startPos=");
        Serial.println(startPos);
        delay(250);
    }

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW); // Enable driver (active LOW on CNC shield)

    stepper.setMaxSpeed(2000.0f);
    stepper.setSpeed(stepperSpeedStepsPerSec);

    Serial.print("Stepper enabled, speed (steps/s): ");
    Serial.println(stepperSpeedStepsPerSec);
    Serial.print("Servo stepping: step=");
    Serial.print(servoStep);
    Serial.print(" deg, interval=");
    Serial.print(servoUpdateIntervalMs);
    Serial.println(" ms");
    Serial.println("Sequence start: Servo 1 moving up");
}

void updateServoSequence()
{
    const unsigned long now = millis();
    if (now - lastServoUpdateMs < servoUpdateIntervalMs)
    {
        return;
    }
    lastServoUpdateMs = now;

    currentPos += direction * servoStep;

    if (direction > 0 && currentPos >= maxPos)
    {
        currentPos = maxPos;
        direction = -1;
        Serial.print("Servo ");
        Serial.print(activeServo + 1);
        Serial.print(" reached maxPos ");
        Serial.print(maxPos);
        Serial.println(", moving down");
    }
    else if (direction < 0 && currentPos <= startPos)
    {
        currentPos = startPos;
        direction = 1;

        Serial.print("Servo ");
        Serial.print(activeServo + 1);
        Serial.print(" returned to startPos ");
        Serial.println(startPos);

        // This servo finished its full cycle, move to the next one.
        activeServo = (activeServo + 1) % SERVO_COUNT;

        Serial.print("Next: Servo ");
        Serial.print(activeServo + 1);
        Serial.println(" moving up");
    }

    servos[activeServo].write(currentPos);
}

void loop()
{
    // Keep stepper moving all the time.
    stepper.runSpeed();

    // Move one servo at a time in sequence.
    updateServoSequence();

    const unsigned long now = millis();
    if (now - lastStatusPrintMs >= 1000)
    {
        lastStatusPrintMs = now;
        Serial.print("Status | activeServo=");
        Serial.print(activeServo + 1);
        Serial.print(" pos=");
        Serial.print(currentPos);
        Serial.print(" dir=");
        Serial.print(direction > 0 ? "up" : "down");
        Serial.print(" stepperSpeed=");
        Serial.println(stepperSpeedStepsPerSec);
    }
}
