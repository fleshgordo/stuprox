#include <Arduino.h>

/*
  DFRobot Quad Motor Driver Shield (DRI0039) - M1 test
  Motor connected to M1 terminal.

  Verified pin mapping from DFRobot wiki:
  - M1 direction pin: D4
  - M1 speed (PWM) pin: D3

    Serial commands (115200 baud, newline):
    - home      : drive in homing direction until switch is hit (set zero point)
    - start     : run motor forward at full power
    - stop      : stop motor
    - move f N  : move forward N logical steps at full power
    - move b N  : move backward N logical steps at full power
    - status    : print current state
    - help      : print commands
*/

const uint8_t M1_DIR_PIN = 4;
const uint8_t M1_PWM_PIN = 3;

const uint8_t HOME_SWITCH_PIN = 2; // Limit switch to GND, uses INPUT_PULLUP
const int HOMING_SPEED = -210;     // Negative direction toward the switch
const uint32_t HOMING_TIMEOUT_MS = 25000;
const bool AUTO_HOME_ON_BOOT = false;
const int FULL_POWER = 255;
const uint16_t STEP_TIME_MS = 10; // One logical step duration

bool motorRunning = false;
bool homed = false;

void setM1(int speedValue)
{
    // speedValue range: -255..255
    speedValue = constrain(speedValue, -255, 255);

    if (speedValue == 0)
    {
        analogWrite(M1_PWM_PIN, 0);
        return;
    }

    // According to DFRobot DRI0039 docs:
    // Forward for M1: DIR LOW, Backward for M1: DIR HIGH
    if (speedValue > 0)
    {
        digitalWrite(M1_DIR_PIN, LOW);
        analogWrite(M1_PWM_PIN, speedValue);
    }
    else
    {
        digitalWrite(M1_DIR_PIN, HIGH);
        analogWrite(M1_PWM_PIN, -speedValue);
    }
}

bool homeSwitchHit()
{
    return digitalRead(HOME_SWITCH_PIN) == LOW;
}

void stopMotor()
{
    motorRunning = false;
    setM1(0);
}

void startMotor()
{
    if (!homed)
    {
        Serial.println("Refusing start: run 'home' first.");
        return;
    }

    motorRunning = true;
    setM1(FULL_POWER);
    Serial.println("Motor started at full power.");
}

void printStatus()
{
    Serial.print("homed=");
    Serial.print(homed ? "yes" : "no");
    Serial.print(", running=");
    Serial.print(motorRunning ? "yes" : "no");
    Serial.print(", switch=");
    Serial.println(homeSwitchHit() ? "HIT" : "open");
}

void printHelp()
{
    Serial.println("Commands: home | start | stop | move f N | move b N | status | help");
}

void moveBySteps(bool forward, int steps)
{
    if (!homed)
    {
        Serial.println("Refusing move: run 'home' first.");
        return;
    }

    if (steps <= 0)
    {
        Serial.println("Move ignored: steps must be > 0.");
        return;
    }

    int signedSpeed = forward ? FULL_POWER : -FULL_POWER;

    Serial.print("Move ");
    Serial.print(forward ? "forward " : "backward ");
    Serial.print(steps);
    Serial.println(" step(s)...");

    uint32_t moveDurationMs = (uint32_t)steps * STEP_TIME_MS;
    uint32_t t0 = millis();
    setM1(signedSpeed);

    while (millis() - t0 < moveDurationMs)
    {
        // If homing switch is hit while moving toward home, stop immediately.
        if (signedSpeed < 0 && homeSwitchHit())
        {
            stopMotor();
            Serial.println("Move aborted: home switch hit.");
            return;
        }
        delay(1);
    }

    stopMotor();
    Serial.println("Move done.");
}

void runHoming()
{
    Serial.println("Homing start...");
    stopMotor();

    // If switch is already pressed, back off slightly first.
    if (homeSwitchHit())
    {
        Serial.println("Switch already HIT, backing off...");
        setM1(-HOMING_SPEED);
        uint32_t backoffStart = millis();
        while (homeSwitchHit())
        {
            if (millis() - backoffStart > 1500)
            {
                stopMotor();
                Serial.println("Backoff timeout, check switch wiring/mechanics.");
                return;
            }
            delay(5);
        }
        stopMotor();
        delay(100);
    }

    setM1(HOMING_SPEED);
    uint32_t t0 = millis();
    while (!homeSwitchHit())
    {
        if (millis() - t0 > HOMING_TIMEOUT_MS)
        {
            stopMotor();
            Serial.println("Homing timeout. Switch not detected.");
            homed = false;
            return;
        }
        delay(5);
    }

    stopMotor();
    homed = true;
    Serial.println("Homing done. Zero point set.");
}

void setup()
{
    pinMode(M1_DIR_PIN, OUTPUT);
    pinMode(M1_PWM_PIN, OUTPUT);
    pinMode(HOME_SWITCH_PIN, INPUT_PULLUP);

    setM1(0);

    Serial.begin(115200);
    delay(300);

    Serial.println("DFRobot DRI0039 M1 serial control ready");
    printHelp();
    printStatus();

    if (AUTO_HOME_ON_BOOT)
    {
        runHoming();
    }
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

    if (cmd == "start")
    {
        startMotor();
    }
    else if (cmd == "stop")
    {
        stopMotor();
        Serial.println("Motor stopped.");
    }
    else if (cmd == "home")
    {
        runHoming();
    }
    else if (cmd.startsWith("move "))
    {
        int firstSpace = cmd.indexOf(' ');
        int secondSpace = cmd.indexOf(' ', firstSpace + 1);

        if (secondSpace < 0)
        {
            Serial.println("Usage: move f N  |  move b N");
            return;
        }

        String dir = cmd.substring(firstSpace + 1, secondSpace);
        String countText = cmd.substring(secondSpace + 1);
        countText.trim();
        int steps = countText.toInt();

        if (dir == "f" || dir == "forward")
        {
            moveBySteps(true, steps);
        }
        else if (dir == "b" || dir == "backward")
        {
            moveBySteps(false, steps);
        }
        else
        {
            Serial.println("Usage: move f N  |  move b N");
        }
    }
    else if (cmd == "status")
    {
        printStatus();
    }
    else if (cmd == "help")
    {
        printHelp();
    }
    else
    {
        Serial.print("Unknown command: ");
        Serial.println(cmd);
        printHelp();
    }
}

void loop()
{
    handleSerial();
}
