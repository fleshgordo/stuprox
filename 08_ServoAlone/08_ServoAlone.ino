#include <Servo.h>

/*  This sketch controls a servo from 0 to 180degrees and back
 *  Adapt to your needs
 *  
 *  Wiring: 
 *  Power supply +5v -> Servo + pin (red cable)
 *  Power supply GND -> CNC Shield GND pin
 *  Servo Data pin (orange or yellow) -> SDA on CNC shield (pin A5 on arduino)
 *  Servo GND -> GND on CNC Shield
 */
 
Servo servo;

const byte servoPin = A5; // this is connected to SDA pin on CNC shield
int pos = 0;

void setup()
{
   Serial.begin(115200);
   servo.attach(servoPin);
}

void loop()
{
  for (pos = 0; pos <= 180; pos++) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
    //Serial.println(pos);          // uncomment to see pos in serial monitor
  }

  for (pos = 180; pos >= 0; pos--) { // goes from 180 degrees to 0 degrees
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
    //Serial.println(pos);
  }
 
}
