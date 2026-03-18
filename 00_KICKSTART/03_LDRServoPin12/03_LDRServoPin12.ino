/*
  03_LDRServoPin12.ino
  - LDR ersetzt das Potentiometer
  - Mini Servo Signal auf Pin 12

  Verdrahtung (Spannungsteiler):
  5V --- LDR ---+--- 10k --- GND
                |
                +--- A0
*/

#include <Servo.h>

const int LDR_PIN = A0;
const int SERVO_PIN = 12;
const unsigned long PRINT_INTERVAL_MS = 100;

Servo servo;
unsigned long lastPrintMs = 0;

void setup()
{
  Serial.begin(115200);
  servo.attach(SERVO_PIN);
  Serial.println("KICKSTART 03: LDR + Servo an Pin 12");
}

void loop()
{
  int raw = analogRead(LDR_PIN);

  // Je nach Aufbau ggf. invertieren: map(raw, 0, 1023, 180, 0)
  int angle = map(raw, 0, 1023, 0, 180);
  angle = constrain(angle, 0, 180);

  servo.write(angle);

  unsigned long now = millis();
  if (now - lastPrintMs >= PRINT_INTERVAL_MS)
  {
    lastPrintMs = now;
    Serial.print("LDR raw: ");
    Serial.print(raw);
    Serial.print(" | Servo angle: ");
    Serial.println(angle);
  }
}
