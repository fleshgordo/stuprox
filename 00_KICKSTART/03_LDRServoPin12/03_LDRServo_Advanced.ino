/*
  03_LDRServo_Advanced.ino
  - LDR ersetzt das Potentiometer
  - Mini Servo Signal auf Pin 12
  - Auto-Kalibrierung fuer 5 Sekunden beim Start

  Verdrahtung (Spannungsteiler):
  5V --- LDR ---+--- 10k --- GND
                |
                +--- A0
*/

#include <Servo.h>

const int LDR_PIN = A0;
const int SERVO_PIN = 12;
const unsigned long PRINT_INTERVAL_MS = 100;
const unsigned long CALIBRATION_MS = 5000;
const int MIN_CALIBRATION_SPAN = 20;

Servo servo;
unsigned long lastPrintMs = 0;
int calibratedMin = 1023;
int calibratedMax = 0;

void calibrateLdr()
{
    Serial.println("Kalibrierung startet (5s): Sensor bewegen (hell/dunkel)...");

    unsigned long startMs = millis();
    while (millis() - startMs < CALIBRATION_MS)
    {
        int raw = analogRead(LDR_PIN);

        if (raw < calibratedMin)
        {
            calibratedMin = raw;
        }
        if (raw > calibratedMax)
        {
            calibratedMax = raw;
        }

        // Visuelles Feedback waehrend der Kalibrierung.
        int previewAngle = map(raw, 0, 1023, 0, 180);
        previewAngle = constrain(previewAngle, 0, 180);
        servo.write(previewAngle);

        delay(5);
    }

    if (calibratedMax - calibratedMin < MIN_CALIBRATION_SPAN)
    {
        calibratedMin = 0;
        calibratedMax = 1023;
        Serial.println("Kalibrierung ungueltig (zu kleiner Bereich) -> Fallback 0..1023");
    }

    Serial.print("Kalibrierung fertig. min=");
    Serial.print(calibratedMin);
    Serial.print(" max=");
    Serial.println(calibratedMax);
}

void setup()
{
    Serial.begin(115200);
    servo.attach(SERVO_PIN);
    Serial.println("KICKSTART 03 Advanced: LDR + Servo mit Auto-Kalibrierung");

    calibrateLdr();
}

void loop()
{
    int raw = analogRead(LDR_PIN);

    // Je nach Aufbau ggf. invertieren: map(rawClamped, calibratedMin, calibratedMax, 180, 0)
    int rawClamped = constrain(raw, calibratedMin, calibratedMax);
    int angle = map(rawClamped, calibratedMin, calibratedMax, 0, 180);
    angle = constrain(angle, 0, 180);

    servo.write(angle);

    unsigned long now = millis();
    if (now - lastPrintMs >= PRINT_INTERVAL_MS)
    {
        lastPrintMs = now;
        Serial.print("LDR raw: ");
        Serial.print(raw);
        Serial.print(" (min/max: ");
        Serial.print(calibratedMin);
        Serial.print("/");
        Serial.print(calibratedMax);
        Serial.print(")");
        Serial.print(" | Servo angle: ");
        Serial.println(angle);
    }
}
