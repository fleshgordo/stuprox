/*
  Two steppers with opposing speed profiles (AccelStepper).

  Behavior:
  - Both motors run continuously.
  - One motor runs FAST while the other runs SLOW.
  - After a time interval, profiles swap (FAST<->SLOW).

  Control options:
  - Serial commands (default)
  - Optional one potentiometer on A0

  CNC shield style pins used in this repo:
  - X: STEP=2, DIR=5
  - Y: STEP=3, DIR=6
  - ENABLE=8 (active LOW)

  Potentiometer:
  - A0 controls RANGE between fast and slow speeds
  - Mid speed stays based on your serial values
*/

#include <Arduino.h>
#include <AccelStepper.h>

#define STEP_X 2
#define DIR_X 5
#define STEP_Y 3
#define DIR_Y 6
#define ENABLE_PIN 8

#define POT_RANGE_PIN A0

AccelStepper stepperX(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepperY(AccelStepper::DRIVER, STEP_Y, DIR_Y);

float fastSpeed = 800.0f; // steps per second
float slowSpeed = 50.0f; // steps per second
float minSpeed = 10.0f;
float maxSpeed = 2000.0f;

bool usePots = false;
bool fastOnX = true; // profile state: true => X fast / Y slow
int xDirSign = 1;    // 1 = forward, -1 = reverse
int yDirSign = -1;   // default opposite to X
unsigned long swapIntervalMs = 10000;
unsigned long lastSwapMs = 0;
unsigned long lastStatusMs = 0;
const unsigned long STATUS_MS = 1000;

const char *dirName(int sign)
{
    return (sign >= 0) ? "fwd" : "rev";
}

void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  fast <value>      e.g. fast 1200");
    Serial.println("  slow <value>      e.g. slow 250");
    Serial.println("  interval <ms>     e.g. interval 2000");
    Serial.println("  pots on|off       use A0 to control speed range");
    Serial.println("  swap              swap profiles now");
    Serial.println("  xdir toggle");
    Serial.println("  ydir toggle");
    Serial.println("  status");
    Serial.println("  help");
}

float clampSpeed(float s)
{
    if (s < minSpeed)
        return minSpeed;
    if (s > maxSpeed)
        return maxSpeed;
    return s;
}

void normalizeFastSlow()
{
    if (fastSpeed < slowSpeed)
    {
        float tmp = fastSpeed;
        fastSpeed = slowSpeed;
        slowSpeed = tmp;
    }
}

void readPotsIfEnabled()
{
    if (!usePots)
    {
        return;
    }

    int v = analogRead(POT_RANGE_PIN);

    float center = (fastSpeed + slowSpeed) * 0.5f;
    float maxHalfRange = min(center - minSpeed, maxSpeed - center);
    if (maxHalfRange < 0.0f)
    {
        maxHalfRange = 0.0f;
    }

    float halfRange = map(v, 0, 1023, 0, (int)maxHalfRange);

    fastSpeed = clampSpeed(center + halfRange);
    slowSpeed = clampSpeed(center - halfRange);
    normalizeFastSlow();
}

void applySpeeds()
{
    float xMag = fastOnX ? fastSpeed : slowSpeed;
    float yMag = fastOnX ? slowSpeed : fastSpeed;

    stepperX.setSpeed((float)xDirSign * xMag);
    stepperY.setSpeed((float)yDirSign * yMag);
}

void printStatus()
{
    Serial.print("mode=");
    Serial.print(usePots ? "pots" : "serial");
    Serial.print(" | profile=");
    Serial.print(fastOnX ? "Xfast_Yslow" : "Xslow_Yfast");
    Serial.print(" | fast=");
    Serial.print(fastSpeed);
    Serial.print(" | slow=");
    Serial.print(slowSpeed);
    Serial.print(" | interval=");
    Serial.print(swapIntervalMs);
    Serial.print(" | xdir=");
    Serial.print(dirName(xDirSign));
    Serial.print(" | ydir=");
    Serial.println(dirName(yDirSign));
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

    if (cmd.length() == 0)
    {
        return;
    }

    if (cmd.startsWith("fast "))
    {
        fastSpeed = clampSpeed(cmd.substring(5).toFloat());
        normalizeFastSlow();
        Serial.print("fast set to ");
        Serial.println(fastSpeed);
        return;
    }

    if (cmd.startsWith("slow "))
    {
        slowSpeed = clampSpeed(cmd.substring(5).toFloat());
        normalizeFastSlow();
        Serial.print("slow set to ");
        Serial.println(slowSpeed);
        return;
    }

    if (cmd.startsWith("interval "))
    {
        long newInterval = cmd.substring(9).toInt();
        if (newInterval < 200)
            newInterval = 200;
        swapIntervalMs = (unsigned long)newInterval;
        Serial.print("interval set to ");
        Serial.println(swapIntervalMs);
        return;
    }

    if (cmd == "pots on")
    {
        usePots = true;
        Serial.println("pot mode ON");
        return;
    }

    if (cmd == "pots off")
    {
        usePots = false;
        Serial.println("pot mode OFF");
        return;
    }

    if (cmd == "swap")
    {
        fastOnX = !fastOnX;
        lastSwapMs = millis();
        Serial.println("profiles swapped");
        return;
    }

    if (cmd == "xdir toggle")
    {
        xDirSign = -xDirSign;
        Serial.print("xdir -> ");
        Serial.println(dirName(xDirSign));
        return;
    }

    if (cmd == "ydir toggle")
    {
        yDirSign = -yDirSign;
        Serial.print("ydir -> ");
        Serial.println(dirName(yDirSign));
        return;
    }

    if (cmd == "status")
    {
        printStatus();
        return;
    }

    if (cmd == "help")
    {
        printHelp();
        return;
    }

    Serial.print("Unknown command: ");
    Serial.println(cmd);
    printHelp();
}

void setup()
{
    Serial.begin(115200);

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);

    pinMode(POT_RANGE_PIN, INPUT);

    stepperX.setMaxSpeed(maxSpeed);
    stepperY.setMaxSpeed(maxSpeed);

    normalizeFastSlow();
    applySpeeds();

    lastSwapMs = millis();
    lastStatusMs = millis();

    Serial.println("Two steppers opposing speed profiles started.");
    printHelp();
    printStatus();
}

void loop()
{
    handleSerial();
    readPotsIfEnabled();

    unsigned long now = millis();
    if (now - lastSwapMs >= swapIntervalMs)
    {
        fastOnX = !fastOnX;
        lastSwapMs = now;
    }

    applySpeeds();

    stepperX.runSpeed();
    stepperY.runSpeed();

    if (now - lastStatusMs >= STATUS_MS)
    {
        lastStatusMs = now;
        printStatus();
    }
}
