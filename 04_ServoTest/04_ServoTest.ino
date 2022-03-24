#include <Servo.h>

/*  This sketch controls a servo from 0 to 180degrees and back
    Adapt to your needs

    Wiring:
    Power supply +5v -> Servo + pin (red cable)
    Power supply GND -> CNC Shield GND pin
    Servo Data pin (orange or yellow) -> SpinEn on CNC shield
    Servo GND -> GND on CNC Shield
*/

Servo servo;

const byte servoPin = 12; // connect to Spindle enable pin (SpinEn) on CNC shield.
int pos = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting postplotter ... Faster your seatbelts! ");
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("");

  servo.attach(servoPin);
}

void loop()
{
  for (pos = 0; pos <= 180; pos++)
  { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos); // tell servo to go to position in variable 'pos'
    delay(15);        // waits 15ms for the servo to reach the position
  }

  for (pos = 180; pos >= 0; pos--)
  { // goes from 180 degrees to 0 degrees
    servo.write(pos); // tell servo to go to position in variable 'pos'
    delay(15);        // waits 15ms for the servo to reach the position
  }
}
