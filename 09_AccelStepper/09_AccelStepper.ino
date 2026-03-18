/*
    09_AccelStepper.ino

  Basic AccelStepper example for CNC Shield X/Y slots.
  - X slot: STEP 2, DIR 5
  - Y slot: STEP 3, DIR 6
  - Shared ENABLE: 8 (active LOW on most CNC shields)

  Features:
  - Non-blocking movement with AccelStepper::run()
  - Shared enable/disable control
  - Simple repeating XY motion pattern
  - Serial commands:
      E  -> enable drivers
      D  -> disable drivers
      R  -> reset position to (0,0)
*/

#include <AccelStepper.h>

// CNC Shield (UNO) pins
const uint8_t STEP_X_PIN = 2;
const uint8_t DIR_X_PIN = 5;
const uint8_t STEP_Y_PIN = 3;
const uint8_t DIR_Y_PIN = 6;
const uint8_t ENABLE_PIN = 8;

// -----------------------------------------------------------------------------
// TUNING GUIDE (units used by AccelStepper)
// speed        = steps/second
// acceleration = steps/second^2
//
// With 200-step motor and 1/4 microstepping -> 800 steps/rev.
// Example: 1200 steps/s ~= 1.5 rev/s ~= 90 RPM
//
// Suggested starting presets:
// - Conservative: speed 600-900,  accel 200-500   (very safe)
// - Balanced:     speed 1000-1500, accel 500-1200 (recommended)
// - Aggressive:   speed 1800-2600, accel 1500-3500 (may skip steps)
// -----------------------------------------------------------------------------
const float MAX_SPEED_X = 1200.0f;
const float MAX_SPEED_Y = 1200.0f;
const float ACCEL_X = 600.0f;
const float ACCEL_Y = 600.0f;

AccelStepper stepperX(AccelStepper::DRIVER, STEP_X_PIN, DIR_X_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, STEP_Y_PIN, DIR_Y_PIN);

bool driversEnabled = false;
unsigned long lastMoveSwitchMs = 0;
const unsigned long PAUSE_BETWEEN_MOVES_MS = 250;

// Basic demo path in steps (X, Y)
const long targets[][2] = {
    {0, 0},
    {800, 0},
    {800, 800},
    {0, 800},
    {0, 0},
    {-600, 600},
    {600, -600},
    {0, 0}};
const size_t targetCount = sizeof(targets) / sizeof(targets[0]);
size_t targetIndex = 0;

void enableDrivers()
{
    digitalWrite(ENABLE_PIN, LOW); // active LOW
    driversEnabled = true;
    Serial.println("Drivers ENABLED");
}

void disableDrivers()
{
    digitalWrite(ENABLE_PIN, HIGH); // disable outputs
    driversEnabled = false;
    Serial.println("Drivers DISABLED");
}

void moveToTarget(size_t idx)
{
    stepperX.moveTo(targets[idx][0]);
    stepperY.moveTo(targets[idx][1]);

    Serial.print("Target[");
    Serial.print(idx);
    Serial.print("] -> X:");
    Serial.print(targets[idx][0]);
    Serial.print(" Y:");
    Serial.println(targets[idx][1]);
}

void setupSteppers()
{
    stepperX.setMaxSpeed(MAX_SPEED_X);
    stepperX.setAcceleration(ACCEL_X);

    stepperY.setMaxSpeed(MAX_SPEED_Y);
    stepperY.setAcceleration(ACCEL_Y);
}

void handleSerial()
{
    if (!Serial.available())
        return;

    char c = toupper(Serial.read());
    if (c == 'E')
    {
        enableDrivers();
    }
    else if (c == 'D')
    {
        disableDrivers();
    }
    else if (c == 'R')
    {
        stepperX.setCurrentPosition(0);
        stepperY.setCurrentPosition(0);
        targetIndex = 0;
        moveToTarget(targetIndex);
        Serial.println("Position reset to (0,0)");
    }
}

void setup()
{
    Serial.begin(115200);

    Serial.println("");
    Serial.println(" ____ _____ _   _ ______  __");
    Serial.println("/ ___|_   _| | | |  _ \\ \\/ /");
    Serial.println("\\___ \\ | | | | | | |_) \\  /");
    Serial.println(" ___) || | | |_| |  __//  \\");
    Serial.println("|____/ |_|  \\___/|_|  /_/\\_\\");
    Serial.println("");

    pinMode(ENABLE_PIN, OUTPUT);
    disableDrivers();

    setupSteppers();

    Serial.println("\n=== AccelStepper Basic XY (CNC Shield) ===");
    Serial.println("Pins: X(STEP2,DIR5), Y(STEP3,DIR6), EN(8)");
    Serial.println("Send E to enable, D to disable, R to reset");

    enableDrivers();
    moveToTarget(targetIndex);
}

void loop()
{
    static bool reachPrinted = false;

    handleSerial();

    if (!driversEnabled)
        return;

    stepperX.run();
    stepperY.run();

    bool xDone = (stepperX.distanceToGo() == 0);
    bool yDone = (stepperY.distanceToGo() == 0);

    if (xDone && yDone)
    {
        if (!reachPrinted)
        {
            Serial.print("Reached[");
            Serial.print(targetIndex);
            Serial.print("] at X:");
            Serial.print(stepperX.currentPosition());
            Serial.print(" Y:");
            Serial.println(stepperY.currentPosition());
            reachPrinted = true;
        }

        if (millis() - lastMoveSwitchMs >= PAUSE_BETWEEN_MOVES_MS)
        {
            targetIndex = (targetIndex + 1) % targetCount;
            moveToTarget(targetIndex);
            lastMoveSwitchMs = millis();
            reachPrinted = false;
        }
    }
    else
    {
        lastMoveSwitchMs = millis();
        reachPrinted = false;
    }
}
