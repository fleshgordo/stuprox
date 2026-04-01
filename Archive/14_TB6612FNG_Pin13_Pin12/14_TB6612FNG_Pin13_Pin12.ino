/*
  TB6612FNG single motor test

  Pin mapping:
    - DIR  -> D12 -> CNC shield SpinEN
    - PWM  -> D9 ->  CNC shield X-

  Sequence:
    1) Forward at current speed (2s)
  2) Stop (1s)
    3) Backward at current speed (2s)
  4) Stop (1s)

    Serial commands:
    - speed N   (0..255)
    - status
    - help
*/

#include <Arduino.h>

const uint8_t DIR_PIN = 12;
const uint8_t PWM_PIN = 9;

int motorSpeed = 90;

enum MotionPhase
{
    PHASE_FORWARD = 0,
    PHASE_STOP_1 = 1,
    PHASE_BACKWARD = 2,
    PHASE_STOP_2 = 3
};

MotionPhase phase = PHASE_FORWARD;
unsigned long phaseStartMs = 0;

const unsigned long FORWARD_TIME_MS = 2000;
const unsigned long STOP_TIME_MS = 1000;
const unsigned long BACKWARD_TIME_MS = 2000;

void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  speed N   (0..255)");
    Serial.println("  status");
    Serial.println("  help");
}

void printStatus()
{
    Serial.print("Speed=");
    Serial.print(motorSpeed);
    Serial.print(" | Phase=");
    if (phase == PHASE_FORWARD)
    {
        Serial.println("FORWARD");
    }
    else if (phase == PHASE_BACKWARD)
    {
        Serial.println("BACKWARD");
    }
    else
    {
        Serial.println("STOP");
    }
}

void setMotor(int speedValue)
{
    speedValue = constrain(speedValue, -255, 255);

    if (speedValue == 0)
    {
        analogWrite(PWM_PIN, 0);
        return;
    }

    if (speedValue > 0)
    {
        digitalWrite(DIR_PIN, HIGH);
        analogWrite(PWM_PIN, speedValue);
    }
    else
    {
        digitalWrite(DIR_PIN, LOW);
        analogWrite(PWM_PIN, -speedValue);
    }
}

void setup()
{
    pinMode(DIR_PIN, OUTPUT);
    pinMode(PWM_PIN, OUTPUT);

    setMotor(0);

    Serial.begin(115200);
    delay(200);
    Serial.println("TB6612FNG test start (DIR=D12, PWM=D11)");
    Serial.println("Default speed = 90");
    printHelp();

    phase = PHASE_FORWARD;
    phaseStartMs = millis();
    setMotor(motorSpeed);
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

    if (cmd.startsWith("speed "))
    {
        int newSpeed = cmd.substring(6).toInt();
        newSpeed = constrain(newSpeed, 0, 255);
        motorSpeed = newSpeed;

        if (phase == PHASE_FORWARD)
        {
            setMotor(motorSpeed);
        }
        else if (phase == PHASE_BACKWARD)
        {
            setMotor(-motorSpeed);
        }

        Serial.print("Speed set to ");
        Serial.println(motorSpeed);
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

void loop()
{
    handleSerial();
    unsigned long now = millis();

    if (phase == PHASE_FORWARD && (now - phaseStartMs >= FORWARD_TIME_MS))
    {
        phase = PHASE_STOP_1;
        phaseStartMs = now;
        Serial.println("Stop");
        setMotor(0);
    }
    else if (phase == PHASE_STOP_1 && (now - phaseStartMs >= STOP_TIME_MS))
    {
        phase = PHASE_BACKWARD;
        phaseStartMs = now;
        Serial.println("Backward");
        setMotor(-motorSpeed);
    }
    else if (phase == PHASE_BACKWARD && (now - phaseStartMs >= BACKWARD_TIME_MS))
    {
        phase = PHASE_STOP_2;
        phaseStartMs = now;
        Serial.println("Stop");
        setMotor(0);
    }
    else if (phase == PHASE_STOP_2 && (now - phaseStartMs >= STOP_TIME_MS))
    {
        phase = PHASE_FORWARD;
        phaseStartMs = now;
        Serial.println("Forward");
        setMotor(motorSpeed);
    }
}
