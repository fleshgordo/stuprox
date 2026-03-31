#/*******************************************************************************
# Wiring & Purpose
#
# - Purpose: tiny servo calibration helper. Use a potentiometer to position a
#   servo and set pulse-width endpoints so the servo can be driven accurately
#   across its safe mechanical travel.
#
# - Wiring (typical Arduino UNO / AVR):
#   * Servo signal (control) -> Arduino digital pin 9 (defined by `servoPin`).
#   * Servo V+ -> 5V supply (for higher-torque servos use a separate 5V supply).
#   * Servo GND -> Arduino GND (and common with external supply ground).
#   * Potentiometer: middle (wiper) -> A0 (`potPin`), one end -> 5V, other end -> GND.
#
# - Usage: open serial monitor at 115200, move the pot to the mechanical
#   extremes of the servo, and press 'm' to set MIN or 'x' to set MAX.
#   Use 't' to run a test sweep and 's' to show current values.
#
# Notes:
# - This sketch uses `writeMicroseconds(us)` to control the servo pulse width
#   directly (more precise than angle mapping). Strings printed with `F()` are
#   stored in flash to save SRAM on AVR boards.
#*******************************************************************************/

#include <Servo.h>

const int potPin = A0;
const int servoPin = 9;

Servo myServo;
int minPulse = 544;  // safe defaults (microseconds) — many servos accept ~544..2400
int maxPulse = 2400; // adjust after calibration if needed

unsigned long lastPrint = 0;
const unsigned long PRINT_INTERVAL = 250;

void printHelp()
{
  Serial.println(F("Commands:"));
  Serial.println(F("  m  -> record current position as MIN (0°)"));
  Serial.println(F("  x  -> record current position as MAX (180°)"));
  Serial.println(F("  s  -> show stored min/max pulse widths"));
  Serial.println(F("  r  -> reset to defaults"));
  Serial.println(F("  t  -> run test sweep 0->180->0"));
  Serial.println(F(""));
  Serial.println(F("Procedure: move pot so servo is at one mechanical extreme, send 'm' or 'x' to save."));
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  myServo.attach(servoPin);

  // Use compiled-in defaults; user can adjust during runtime with 'm'/'x'
  Serial.println(F("Servo calibration helper"));
  printHelp();
  Serial.print(F("Using minPulse="));
  Serial.print(minPulse);
  Serial.print(F("  maxPulse="));
  Serial.println(maxPulse);
}

void loop()
{
  int pot = analogRead(potPin); // 0..1023
  // Map pot position directly to a pulse width in microseconds
  int pulse = map(pot, 0, 1023, minPulse, maxPulse);
  // Directly set the servo pulse width for precise control
  myServo.writeMicroseconds(pulse);

  if (Serial.available())
  {
    char c = Serial.read();
    if (c == 'm' || c == 'M')
    {
      minPulse = pulse;
      Serial.print(F("Set MIN pulse = "));
      Serial.println(minPulse);
    }
    else if (c == 'x' || c == 'X')
    {
      maxPulse = pulse;
      Serial.print(F("Set MAX pulse = "));
      Serial.println(maxPulse);
    }
    else if (c == 's' || c == 'S')
    {
      Serial.print(F("minPulse="));
      Serial.print(minPulse);
      Serial.print(F("  maxPulse="));
      Serial.println(maxPulse);
    }
    else if (c == 'r' || c == 'R')
    {
      minPulse = 544;
      maxPulse = 2400;
      Serial.println(F("Reset to defaults (RAM only)."));
    }
    else if (c == 't' || c == 'T')
    {
      Serial.println(F("Starting sweep..."));
      for (int a = 0; a <= 180; a += 5)
      {
        int pw = map(a, 0, 180, minPulse, maxPulse);
        myServo.writeMicroseconds(pw);
        delay(20);
      }
      delay(200);
      for (int a = 180; a >= 0; a -= 5)
      {
        int pw = map(a, 0, 180, minPulse, maxPulse);
        myServo.writeMicroseconds(pw);
        delay(20);
      }
      Serial.println(F("Sweep done."));
    }
  }

  if (millis() - lastPrint >= PRINT_INTERVAL)
  {
    lastPrint = millis();
    // `F()` keeps the literal in flash (saves SRAM on AVR boards)
    Serial.print(F("pot="));
    Serial.print(analogRead(potPin));
    Serial.print(F("  pulse="));
    Serial.print(pulse);
    Serial.print(F("  min="));
    Serial.print(minPulse);
    Serial.print(F("  max="));
    Serial.println(maxPulse);
  }
}