/*
  14_2_TB6612.ino

  Advanced non-blocking startup sequence:
  1) Stepper moves +550 steps
  2) Stepper moves -200 steps
  3) DC motor starts (mode: crazy / medium / soft)
  4) While DC runs, stepper walks up/down between target points
  5) After configurable up/down cycles, DC stops
  6) Stepper returns to zero

  Serial commands:
    mode crazy
    mode medium
    mode soft
    restart
    status
    help
*/

#include <Arduino.h>
#include <AccelStepper.h>

// -------------------- DC motor (TB6612) --------------------
const uint8_t DC_DIR_PIN = 12; // SpinEN on CNC shield
const uint8_t DC_PWM_PIN = 9;  // PWM output pin

struct DcMode
{
    const char *name;
    int speed;                   // 0..255
    bool alternateDirection;     // true for crazy mode
    unsigned long toggleEveryMs; // used only when alternateDirection=true
};

const DcMode dcModes[] = {
    {"crazy", 255, true, 140},
    {"medium", 165, false, 0},
    {"soft", 95, false, 0},
    // you can extend the modes here ...
};

// -------------------- Startup/walk parameters --------------------
const long startupForwardSteps = 550;
const long startupBackSteps = 200;

// During DC run, stepper walks between these offsets (relative to base position).
const long walkOffsets[] = {120, -120};
const uint8_t WALK_POINTS = sizeof(walkOffsets) / sizeof(walkOffsets[0]);

const uint8_t DC_MODE_COUNT = sizeof(dcModes) / sizeof(dcModes[0]);
uint8_t activeMode = 1; // 0=crazy, 1=medium, 2=soft

bool dcRunning = false;
int dcDirectionSign = 1;
unsigned long lastDcToggleMs = 0;

// -------------------- Stepper (non-blocking) --------------------
// CNC shield X slot: STEP=2, DIR=5, EN=8
const uint8_t STEP_PIN = 2;
const uint8_t STEP_DIR_PIN = 5;
const uint8_t STEP_ENABLE_PIN = 8;

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, STEP_DIR_PIN);

const float stepperMaxSpeed = 700.0f;
const float stepperAccel = 500.0f;

uint8_t walkIndex = 0;
int upDownCyclesDone = 0;
int upDownCyclesTarget = 3; // set to 2 or 3 (or any value >=1)
long walkBasePos = 0;

// -------------------- Sequence state machine --------------------
enum SequenceState
{
    ST_START_MOVE_550 = 0,
    ST_START_BACK_200 = 1,
    ST_WALK_AND_SPIN = 2,
    ST_RETURN_HOME = 3,
    ST_DONE = 4,
};

SequenceState state = ST_START_MOVE_550;

// Sets DC motor direction and PWM speed (-255..255).
void setDcMotor(int speedValue)
{
    speedValue = constrain(speedValue, -255, 255);

    if (speedValue == 0)
    {
        analogWrite(DC_PWM_PIN, 0);
        return;
    }

    if (speedValue > 0)
    {
        digitalWrite(DC_DIR_PIN, HIGH);
        analogWrite(DC_PWM_PIN, speedValue);
    }
    else
    {
        digitalWrite(DC_DIR_PIN, LOW);
        analogWrite(DC_PWM_PIN, -speedValue);
    }
}

// Starts DC motor using the currently selected mode.
void startDc()
{
    const DcMode &m = dcModes[activeMode];
    dcRunning = true;
    dcDirectionSign = 1;
    lastDcToggleMs = millis();
    setDcMotor(dcDirectionSign * m.speed);

    Serial.print("DC start | mode=");
    Serial.print(m.name);
    Serial.print(" speed=");
    Serial.println(m.speed);
}

// Stops the DC motor immediately.
void stopDc()
{
    dcRunning = false;
    setDcMotor(0);
    Serial.println("DC stop");
}

// Updates optional DC direction toggling for crazy mode.
void updateDc()
{
    if (!dcRunning)
    {
        return;
    }

    const DcMode &m = dcModes[activeMode];
    if (!m.alternateDirection)
    {
        return;
    }

    const unsigned long now = millis();
    if (now - lastDcToggleMs >= m.toggleEveryMs)
    {
        lastDcToggleMs = now;
        dcDirectionSign = -dcDirectionSign;
        setDcMotor(dcDirectionSign * m.speed);
    }
}

// Resets variables and starts the startup sequence from step 1.
void startSequence()
{
    stopDc();

    walkIndex = 0;
    upDownCyclesDone = 0;

    state = ST_START_MOVE_550;
    stepper.moveTo(startupForwardSteps);

    Serial.println("Sequence start");
    Serial.println("Step 1: move +550");
}

// Prints current runtime state and counters.
void printStatus()
{
    Serial.print("state=");
    Serial.print((int)state);
    Serial.print(" stepperPos=");
    Serial.print(stepper.currentPosition());
    Serial.print(" stepperTarget=");
    Serial.print(stepper.targetPosition());
    Serial.print(" dc=");
    Serial.print(dcRunning ? "ON" : "OFF");
    Serial.print(" mode=");
    Serial.print(dcModes[activeMode].name);
    Serial.print(" cycles=");
    Serial.print(upDownCyclesDone);
    Serial.print("/");
    Serial.println(upDownCyclesTarget);
}

// Prints available serial commands.
void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  mode crazy");
    Serial.println("  mode medium");
    Serial.println("  mode soft");
    Serial.println("  restart");
    Serial.println("  status");
    Serial.println("  help");
}

// Handles serial input commands.
void handleSerial()
{
    if (!Serial.available())
    {
        return;
    }

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "mode crazy")
    {
        activeMode = 0;
        Serial.println("Mode set to crazy");
    }
    else if (cmd == "mode medium")
    {
        activeMode = 1;
        Serial.println("Mode set to medium");
    }
    else if (cmd == "mode soft")
    {
        activeMode = 2;
        Serial.println("Mode set to soft");
    }
    else if (cmd == "restart")
    {
        startSequence();
    }
    else if (cmd == "status")
    {
        printStatus();
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

// Advances the non-blocking state machine.
void updateSequence()
{
    if (state == ST_START_MOVE_550)
    {
        if (stepper.distanceToGo() == 0)
        {
            state = ST_START_BACK_200;
            stepper.moveTo(startupForwardSteps - startupBackSteps); // 550 - 200 = 350
            Serial.println("Step 2: back -200");
        }
    }
    else if (state == ST_START_BACK_200)
    {
        if (stepper.distanceToGo() == 0)
        {
            walkBasePos = stepper.currentPosition();
            walkIndex = 0;
            upDownCyclesDone = 0;

            state = ST_WALK_AND_SPIN;
            startDc();

            stepper.moveTo(walkBasePos + walkOffsets[walkIndex]);
            Serial.println("Step 3: DC on + stepper walk up/down");
        }
    }
    else if (state == ST_WALK_AND_SPIN)
    {
        if (stepper.distanceToGo() == 0)
        {
            walkIndex = (walkIndex + 1) % WALK_POINTS;
            stepper.moveTo(walkBasePos + walkOffsets[walkIndex]);

            if (walkIndex == 0)
            {
                upDownCyclesDone++;
                Serial.print("Up/down cycle done: ");
                Serial.println(upDownCyclesDone);

                if (upDownCyclesDone >= upDownCyclesTarget)
                {
                    stopDc();
                    state = ST_RETURN_HOME;
                    stepper.moveTo(0);
                    Serial.println("Step 4: return stepper to zero");
                }
            }
        }
    }
    else if (state == ST_RETURN_HOME)
    {
        if (stepper.distanceToGo() == 0)
        {
            state = ST_DONE;
            Serial.println("Done: stepper at zero, DC stopped");
        }
    }
}

// Initializes pins, stepper settings, serial, and starts the sequence.
void setup()
{
    pinMode(DC_DIR_PIN, OUTPUT);
    pinMode(DC_PWM_PIN, OUTPUT);

    pinMode(STEP_ENABLE_PIN, OUTPUT);
    digitalWrite(STEP_ENABLE_PIN, LOW); // enable stepper driver (active LOW)

    setDcMotor(0);

    stepper.setMaxSpeed(stepperMaxSpeed);
    stepper.setAcceleration(stepperAccel);
    stepper.setCurrentPosition(0);

    Serial.begin(115200);
    delay(150);

    Serial.println("14_2_TB6612 advanced test");
    Serial.print("Default mode: ");
    Serial.println(dcModes[activeMode].name);
    Serial.print("Up/down cycles target: ");
    Serial.println(upDownCyclesTarget);
    printHelp();

    startSequence();
}

// Main loop: serial, DC update, sequence update, stepper run.
void loop()
{
    handleSerial();
    updateDc();
    updateSequence();
    stepper.run(); // non-blocking
}
