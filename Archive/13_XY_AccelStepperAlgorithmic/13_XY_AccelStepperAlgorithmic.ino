/*
  Non-blocking dual-stepper sketch using AccelStepper.

  - X motor rotates continuously.
  - Y motor runs algorithmic motion modes:
    1) sine-like target motion
    2) random walk
    3) random points with varying speeds

  Board wiring follows existing CNC shield examples in this repo:
  - X: DIR=5, STEP=2
  - Y: DIR=6, STEP=3
  - ENABLE/SLEEP=8 (active LOW)

  Microstepping is set externally on driver jumpers.
  Keep this value in sync with the jumper setting.
*/

#include <Arduino.h>
#include <AccelStepper.h>

#define MOTOR_STEPS 200
#define MICROSTEPS 4
#define STEPS_PER_REV (MOTOR_STEPS * MICROSTEPS)

#define DIR_X 5
#define STEP_X 2
#define DIR_Y 6
#define STEP_Y 3
#define ENABLE_PIN 8

AccelStepper stepperX(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepperY(AccelStepper::DRIVER, STEP_Y, DIR_Y);

enum YMode
{
    MODE_SINE = 0,
    MODE_RANDOM_WALK = 1,
    MODE_RANDOM_POINTS = 2,
    MODE_STOP = 3
};

YMode currentMode = MODE_STOP;

const float X_SPEED_DEFAULT = 500.0f;
const float X_SPEED_MIN = 50.0f;
const float X_SPEED_MAX = 2000.0f;
float xSpeedStepsPerSec = X_SPEED_DEFAULT;
const long Y_LIMIT = STEPS_PER_REV * 3L;

const float Y_MAX_SPEED_SINE = 400.0f;
const float Y_ACCEL_SINE = 1000.0f;
const uint16_t SINE_UPDATE_MS = 30;
const float SINE_STEP_RAD = 0.12f;
float sinePhase = 0.0f;

const float Y_ACCEL_RANDOM = 1000.0f;

const uint32_t STATUS_PRINT_MS = 1000;

uint32_t lastStatusMs = 0;
uint32_t lastSineUpdateMs = 0;

const char *modeName(YMode m)
{
    if (m == MODE_SINE)
    {
        return "SINE";
    }
    if (m == MODE_RANDOM_WALK)
    {
        return "RANDOM_WALK";
    }
    if (m == MODE_RANDOM_POINTS)
    {
        return "RANDOM_POINTS";
    }
    return "STOP";
}

void setMode(YMode mode)
{
    currentMode = mode;

    if (currentMode == MODE_SINE)
    {
        digitalWrite(ENABLE_PIN, LOW);
        stepperX.setSpeed(xSpeedStepsPerSec);
        stepperY.setAcceleration(Y_ACCEL_SINE);
        stepperY.setMaxSpeed(Y_MAX_SPEED_SINE);
    }
    else if (currentMode == MODE_RANDOM_WALK || currentMode == MODE_RANDOM_POINTS)
    {
        digitalWrite(ENABLE_PIN, LOW);
        stepperX.setSpeed(xSpeedStepsPerSec);
        stepperY.setAcceleration(Y_ACCEL_RANDOM);
    }
    else
    {
        stepperX.setSpeed(0.0f);
        stepperY.stop();
        digitalWrite(ENABLE_PIN, HIGH);
    }

    Serial.print("Y mode -> ");
    Serial.println(modeName(currentMode));
}

void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  mode sine");
    Serial.println("  mode walk");
    Serial.println("  mode points");
    Serial.println("  mode stop");
    Serial.println("  xspeed <steps_per_sec>   (50..2000)");
    Serial.println("  status");
    Serial.println("  help");
}

void handleSerial()
{
    if (!Serial.available())
    {
        return;
    }

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "mode sine")
    {
        setMode(MODE_SINE);
    }
    else if (cmd == "mode walk" || cmd == "mode random_walk")
    {
        setMode(MODE_RANDOM_WALK);
    }
    else if (cmd == "mode points" || cmd == "mode random_points")
    {
        setMode(MODE_RANDOM_POINTS);
    }
    else if (cmd == "mode stop")
    {
        setMode(MODE_STOP);
        Serial.println("Sketch stopped. Select another mode to run again.");
    }
    else if (cmd.startsWith("xspeed "))
    {
        float requestedSpeed = cmd.substring(7).toFloat();
        if (requestedSpeed < X_SPEED_MIN)
        {
            requestedSpeed = X_SPEED_MIN;
        }
        if (requestedSpeed > X_SPEED_MAX)
        {
            requestedSpeed = X_SPEED_MAX;
        }

        xSpeedStepsPerSec = requestedSpeed;
        if (currentMode != MODE_STOP)
        {
            stepperX.setSpeed(xSpeedStepsPerSec);
        }

        Serial.print("X speed set to ");
        Serial.print(xSpeedStepsPerSec);
        Serial.println(" steps/s");
    }
    else if (cmd == "status")
    {
        Serial.print("Mode=");
        Serial.print(modeName(currentMode));
        Serial.print(" | Xspeed=");
        Serial.println(xSpeedStepsPerSec);
    }
    else if (cmd == "help")
    {
        printHelp();
    }
    else if (cmd.length() > 0)
    {
        Serial.print("Unknown command: ");
        Serial.println(cmd);
        printHelp();
    }
}

void updateYMotion(uint32_t nowMs)
{
    switch (currentMode)
    {
    case MODE_SINE:
    {
        if (nowMs - lastSineUpdateMs >= SINE_UPDATE_MS)
        {
            lastSineUpdateMs = nowMs;

            long target = (long)(sin(sinePhase) * (float)Y_LIMIT);
            stepperY.moveTo(target);
            sinePhase += SINE_STEP_RAD;
            if (sinePhase >= TWO_PI)
            {
                sinePhase -= TWO_PI;
            }
        }
        break;
    }

    case MODE_RANDOM_WALK:
    {
        if (stepperY.distanceToGo() != 0)
        {
            break;
        }

        long delta = random(-STEPS_PER_REV, STEPS_PER_REV + 1);
        long nextTarget = stepperY.currentPosition() + delta;
        nextTarget = constrain(nextTarget, -Y_LIMIT, Y_LIMIT);

        float nextSpeed = (float)random(300, 1401);
        stepperY.setMaxSpeed(nextSpeed);
        stepperY.moveTo(nextTarget);
        break;
    }

    case MODE_RANDOM_POINTS:
    {
        if (stepperY.distanceToGo() != 0)
        {
            break;
        }

        long nextTarget = random(-Y_LIMIT, Y_LIMIT + 1);
        float nextSpeed = (float)random(150, 1801);
        stepperY.setMaxSpeed(nextSpeed);
        stepperY.moveTo(nextTarget);
        break;
    }

    case MODE_STOP:
    default:
        break;
    }
}

void printStatus(uint32_t nowMs)
{
    if (nowMs - lastStatusMs < STATUS_PRINT_MS)
    {
        return;
    }

    lastStatusMs = nowMs;
    Serial.print("Mode=");
    Serial.print(modeName(currentMode));
    Serial.print(" | Xspeed=");
    Serial.print(xSpeedStepsPerSec);
    Serial.print(" | Xpos=");
    Serial.print(stepperX.currentPosition());
    Serial.print(" | Ypos=");
    Serial.print(stepperY.currentPosition());
    Serial.print(" | Ytarget=");
    Serial.println(stepperY.targetPosition());
}

void setup()
{
    Serial.begin(115200);
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);

    randomSeed(analogRead(A0));

    stepperX.setMaxSpeed(2000.0f);
    stepperX.setSpeed(xSpeedStepsPerSec);

    stepperY.setMaxSpeed(Y_MAX_SPEED_SINE);
    stepperY.setAcceleration(Y_ACCEL_SINE);
    stepperY.setCurrentPosition(0);
    stepperY.moveTo(0);

    uint32_t nowMs = millis();
    lastStatusMs = nowMs;
    lastSineUpdateMs = nowMs;

    Serial.println("XY AccelStepper non-blocking sketch started.");
    Serial.println("X = continuous rotation, Y = algorithmic movement.");
    printHelp();
    setMode(MODE_STOP);
}

void loop()
{
    uint32_t nowMs = millis();

    handleSerial();

    if (currentMode == MODE_STOP)
    {
        printStatus(nowMs);
        return;
    }

    stepperX.runSpeed();

    updateYMotion(nowMs);
    stepperY.run();

    printStatus(nowMs);
}
