/*
  02_PotiServoPin12.ino
  - Potentiometer auf A0
  - Mini Servo Signal auf Pin 12

  Hinweis: Beim Poti KEIN Pullup verwenden.
  Verdrahtung als Spannungsteiler:
  - ein Außenpin an 5V
  - anderer Außenpin an GND
  - Mittelpin (Wiper) an A0
*/

#include <Servo.h>

const uint8_t POT_PIN = A0;
const uint8_t SERVO_PIN = 12;
const unsigned long PRINT_INTERVAL_MS = 100;

Servo servo;
unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  servo.attach(SERVO_PIN);
  Serial.println("KICKSTART 02: Poti + Servo an Pin 12");
}

void loop() {
  int raw = analogRead(POT_PIN);
  int angle = map(raw, 0, 1023, 0, 180);
  angle = constrain(angle, 0, 180);

  servo.write(angle);

  unsigned long now = millis();
  if (now - lastPrintMs >= PRINT_INTERVAL_MS) {
    lastPrintMs = now;
    Serial.print("POT raw: ");
    Serial.print(raw);
    Serial.print(" | Servo angle: ");
    Serial.println(angle);
  }
}
