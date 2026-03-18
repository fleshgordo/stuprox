/*
   12_potiswitcher

   Potentiometer on A0 is mechanically snapped into 12 positions.
   This sketch converts the analog value (0..1023) into mode index 0..11
   and demonstrates switching between 12 different LED behaviors.
*/

#include <Arduino.h>

const int POT_PIN = A0;
const int LED_PIN = LED_BUILTIN;

const int MODE_COUNT = 12;
const int STABLE_READS_REQUIRED = 3;

int currentMode = 0;
int candidateMode = 0;
int stableReadCount = 0;

unsigned long previousMillis = 0;
int blinkStep = 0;

int readModeFromPot()
{
    int raw = analogRead(POT_PIN); // 0..1023

    // Split the full ADC range into 12 equal bins (0..11).
    int mode = (static_cast<long>(raw) * MODE_COUNT) / 1024;
    if (mode >= MODE_COUNT)
    {
        mode = MODE_COUNT - 1;
    }
    return mode;
}

void printModeInfo(int mode)
{
    Serial.print("Mode ");
    Serial.print(mode);
    Serial.print(": ");

    switch (mode)
    {
    case 0:
        Serial.println("LED OFF");
        break;
    case 1:
        Serial.println("LED ON");
        break;
    case 2:
        Serial.println("Blink 1000 ms");
        break;
    case 3:
        Serial.println("Blink 500 ms");
        break;
    case 4:
        Serial.println("Blink 250 ms");
        break;
    case 5:
        Serial.println("Blink 125 ms");
        break;
    case 6:
        Serial.println("Double blink pattern");
        break;
    case 7:
        Serial.println("Triple blink pattern");
        break;
    case 8:
        Serial.println("1x short flash every 2 s");
        break;
    case 9:
        Serial.println("2x short flash every 2 s");
        break;
    case 10:
        Serial.println("3x short flash every 2 s");
        break;
    case 11:
        Serial.println("4x short flash every 2 s");
        break;
    default:
        Serial.println("Unknown mode");
        break;
    }
}

void updateStableMode()
{
    int measuredMode = readModeFromPot();

    if (measuredMode == candidateMode)
    {
        if (stableReadCount < 255)
        {
            stableReadCount++;
        }
    }
    else
    {
        candidateMode = measuredMode;
        stableReadCount = 1;
    }

    if (stableReadCount >= STABLE_READS_REQUIRED && currentMode != candidateMode)
    {
        currentMode = candidateMode;
        blinkStep = 0;
        previousMillis = millis();
        printModeInfo(currentMode);
    }
}

void runPatternBlink(unsigned long intervalMs)
{
    unsigned long now = millis();
    if (now - previousMillis >= intervalMs)
    {
        previousMillis = now;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}

void runPatternBurst(int pulses, unsigned long pulseOnMs, unsigned long pulseOffMs, unsigned long pauseMs)
{
    unsigned long now = millis();

    if (blinkStep < pulses * 2)
    {
        bool ledOnPhase = (blinkStep % 2 == 0);
        digitalWrite(LED_PIN, ledOnPhase ? HIGH : LOW);
        unsigned long phaseDuration = ledOnPhase ? pulseOnMs : pulseOffMs;

        if (now - previousMillis >= phaseDuration)
        {
            previousMillis = now;
            blinkStep++;
        }
    }
    else
    {
        digitalWrite(LED_PIN, LOW);
        if (now - previousMillis >= pauseMs)
        {
            previousMillis = now;
            blinkStep = 0;
            digitalWrite(LED_PIN, HIGH);
        }
    }
}

void runCurrentMode()
{
    switch (currentMode)
    {
    case 0:
        digitalWrite(LED_PIN, LOW);
        break;

    case 1:
        digitalWrite(LED_PIN, HIGH);
        break;

    case 2:
        runPatternBlink(1000);
        break;

    case 3:
        runPatternBlink(500);
        break;

    case 4:
        runPatternBlink(250);
        break;

    case 5:
        runPatternBlink(125);
        break;

    case 6:
        runPatternBurst(2, 120, 120, 900);
        break;

    case 7:
        runPatternBurst(3, 90, 90, 1000);
        break;

    case 8:
        runPatternBurst(1, 80, 100, 1800);
        break;

    case 9:
        runPatternBurst(2, 80, 100, 1600);
        break;

    case 10:
        runPatternBurst(3, 80, 100, 1400);
        break;

    case 11:
        runPatternBurst(4, 80, 100, 1200);
        break;

    default:
        digitalWrite(LED_PIN, LOW);
        break;
    }
}

void setup()
{
    pinMode(POT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(115200);
    Serial.println("12-position poti switcher ready");

    currentMode = readModeFromPot();
    candidateMode = currentMode;
    stableReadCount = STABLE_READS_REQUIRED;

    printModeInfo(currentMode);
}

void loop()
{
    updateStableMode();
    runCurrentMode();
}
