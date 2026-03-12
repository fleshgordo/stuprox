/*
  13_AccelStepperMelodyXY.ino

  Plays a melody on both CNC shield steppers (X + Y) using AccelStepper.
  - X slot: STEP 2, DIR 5
  - Y slot: STEP 3, DIR 6
  - Shared ENABLE: 8 (active LOW on common CNC shields)
*/

#include <AccelStepper.h>

const uint8_t STEP_X_PIN = 2;
const uint8_t DIR_X_PIN = 5;
const uint8_t STEP_Y_PIN = 3;
const uint8_t DIR_Y_PIN = 6;
const uint8_t ENABLE_PIN = 8;

AccelStepper stepperX(AccelStepper::DRIVER, STEP_X_PIN, DIR_X_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, STEP_Y_PIN, DIR_Y_PIN);

#define NOTE_R 0
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_G3 196

const uint16_t marioMelodyHz[] = {
    NOTE_E5, NOTE_E5, NOTE_R, NOTE_E5,
    NOTE_R, NOTE_C5, NOTE_E5, NOTE_R,
    NOTE_G5, NOTE_R, NOTE_R, NOTE_R,
    NOTE_G4, NOTE_R,
    NOTE_C5, NOTE_R, NOTE_G4, NOTE_R,
    NOTE_E4, NOTE_R, NOTE_A4, NOTE_B4,
    NOTE_A4, NOTE_G4, NOTE_E5, NOTE_G5,
    NOTE_A4, NOTE_F5, NOTE_G5, NOTE_E5,
    NOTE_C5, NOTE_D5, NOTE_B4, NOTE_R};

const uint8_t marioDurationsDiv[] = {
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 4, 4};

const uint16_t preludeGMelodyHz[] = {
    NOTE_G3, NOTE_D4, NOTE_G4, NOTE_B4,
    NOTE_D5, NOTE_G4, NOTE_B4, NOTE_D5,
    NOTE_G3, NOTE_D4, NOTE_G4, NOTE_B4,
    NOTE_D5, NOTE_G4, NOTE_B4, NOTE_D5,
    NOTE_G3, NOTE_D4, NOTE_G4, NOTE_B4,
    NOTE_D5, NOTE_G4, NOTE_B4, NOTE_D5,
    NOTE_C4, NOTE_G4, NOTE_C5, NOTE_E5,
    NOTE_G4, NOTE_E5, NOTE_C5, NOTE_G4,
    NOTE_D4, NOTE_A4, NOTE_D5, NOTE_FS5,
    NOTE_A4, NOTE_FS5, NOTE_D5, NOTE_A4,
    NOTE_G3, NOTE_D4, NOTE_G4, NOTE_B4,
    NOTE_D5, NOTE_G4, NOTE_B4, NOTE_D5};

const uint8_t preludeGDurationsDiv[] = {
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16};

const uint16_t *currentMelodyHz = marioMelodyHz;
const uint8_t *currentRhythm = marioDurationsDiv;
size_t melodyCount = sizeof(marioMelodyHz) / sizeof(marioMelodyHz[0]);
const char *currentMelodyName = "Mario";

const uint16_t BASE_BEAT_MS = 600;
const uint8_t NOTE_ON_PERCENT = 85;

size_t noteIndex = 0;
bool noteIsOn = false;
bool melodyEnabled = true;
unsigned long notePhaseStartMs = 0;
unsigned long noteOnDurationMs = 0;
unsigned long noteTotalDurationMs = 0;

void startNote(size_t idx);

void enableDrivers()
{
    digitalWrite(ENABLE_PIN, LOW);
    melodyEnabled = true;
    Serial.println("Drivers ENABLED");
}

void disableDrivers()
{
    digitalWrite(ENABLE_PIN, HIGH);
    melodyEnabled = false;
    stepperX.setSpeed(0);
    stepperY.setSpeed(0);
    Serial.println("Drivers DISABLED");
}

void setupSteppers()
{
    stepperX.setMaxSpeed(2500);
    stepperY.setMaxSpeed(2500);
    stepperX.setAcceleration(2000);
    stepperY.setAcceleration(2000);
}

void selectMelody(
    const uint16_t *melody,
    const uint8_t *rhythm,
    size_t count,
    const char *name)
{
    currentMelodyHz = melody;
    currentRhythm = rhythm;
    melodyCount = count;
    currentMelodyName = name;
    noteIndex = 0;
    startNote(noteIndex);

    Serial.print("Melody -> ");
    Serial.println(currentMelodyName);
}

void startNote(size_t idx)
{
    uint16_t frequency = currentMelodyHz[idx];
    uint8_t rhythmValue = currentRhythm[idx];
    if (rhythmValue == 0)
        rhythmValue = 1;

    noteTotalDurationMs = BASE_BEAT_MS * 4UL / rhythmValue;

    noteOnDurationMs = (noteTotalDurationMs * NOTE_ON_PERCENT) / 100;
    notePhaseStartMs = millis();
    noteIsOn = true;

    if (frequency == NOTE_R)
    {
        stepperX.setSpeed(0);
        stepperY.setSpeed(0);
    }
    else
    {
        stepperX.setSpeed(frequency);
        stepperY.setSpeed(-frequency * 0.5f);
    }

    Serial.print("Note[");
    Serial.print(idx);
    Serial.print("] f=");
    Serial.print(frequency);
    Serial.print("Hz dur=");
    Serial.print(noteTotalDurationMs);
    Serial.print("ms theme=");
    Serial.println(currentMelodyName);
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
    setupSteppers();
    enableDrivers();

    Serial.println("=== AccelStepper Melody XY ===");
    Serial.println("Commands: E(enable), D(disable), N(next note), M(Mario), B(PreludeG)");

    startNote(noteIndex);
}

void loop()
{
    if (Serial.available())
    {
        char c = toupper(Serial.read());
        if (c == 'E')
        {
            enableDrivers();
            startNote(noteIndex);
        }
        else if (c == 'D')
        {
            disableDrivers();
        }
        else if (c == 'N')
        {
            noteIndex = (noteIndex + 1) % melodyCount;
            startNote(noteIndex);
        }
        else if (c == 'M')
        {
            selectMelody(
                marioMelodyHz,
                marioDurationsDiv,
                sizeof(marioMelodyHz) / sizeof(marioMelodyHz[0]),
                "Mario");
        }
        else if (c == 'B')
        {
            selectMelody(
                preludeGMelodyHz,
                preludeGDurationsDiv,
                sizeof(preludeGMelodyHz) / sizeof(preludeGMelodyHz[0]),
                "PreludeG");
        }
    }

    if (!melodyEnabled)
        return;

    unsigned long now = millis();

    if (noteIsOn && (now - notePhaseStartMs >= noteOnDurationMs))
    {
        stepperX.setSpeed(0);
        stepperY.setSpeed(0);
        noteIsOn = false;
    }

    if (now - notePhaseStartMs >= noteTotalDurationMs)
    {
        noteIndex = (noteIndex + 1) % melodyCount;
        startNote(noteIndex);
    }

    stepperX.runSpeed();
    stepperY.runSpeed();
}
