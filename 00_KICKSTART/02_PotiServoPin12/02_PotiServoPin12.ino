/*
  02_PotiServoPin12.ino
  - Potentiometer auf A0
  - Mini Servo Signal auf Pin 12

  Poti (3-pin) Anschluss:
  Arduino         Potentiometer
  -------         -------------
  5V    --------> Außenpin 1
  GND   --------> Außenpin 2
  A0    --------> Mittelpin (Wiper)

  Servo (3-wire) Anschluss:
  Arduino         Servo
  -------         -----
  5V    --------> VCC (rot)
  GND   --------> GND (braun/schwarz)
  D12   --------> SIG (orange/gelb)
*/

#include <Servo.h>

const int POT_PIN = A0;
const int SERVO_PIN = 12;
const unsigned long PRINT_INTERVAL_MS = 100;

Servo servo;
unsigned long lastPrintMs = 0;

void setup()
{
  Serial.begin(115200);
  servo.attach(SERVO_PIN);
  Serial.println("KICKSTART 02: Poti + Servo an Pin 12");
}

void loop()
{
  int raw = analogRead(POT_PIN);        // read potentiometer value (ADC)
  int angle = map(raw, 0, 1024, 90, 180); // scale ADC range to servo angle
  angle = constrain(angle, 0, 180);     // keep angle within valid limits

  servo.write(angle);

  unsigned long now = millis();
  if (now - lastPrintMs >= PRINT_INTERVAL_MS)
  {
    lastPrintMs = now;
    Serial.print("raw:");
    Serial.print(raw);
    Serial.print(",angle:");
    Serial.println(angle);
  }
}
