#include <Servo.h>

/*  This sketch controls a servo from 0 to 180degrees and back
    Adapt to your needs. 

    Wiring:
    Power supply +5v -> Servo + pin (red cable)
    Power supply GND -> CNC Shield GND pin
    Servo Data pin (orange or yellow) -> SpinEn on CNC shield
    Servo GND -> GND on CNC Shield
*/

Servo servo;

const byte servo_pin = 12;  // connect to Spindle enable pin (SpinEn) on CNC shield.
float pos = 0;

const int servo_min_ms = 800;
const int servo_max_ms = 2100;

const int servo_min_pos = 55;
const int servo_max_pos = 135;
const int servo_center = 95;
const int range = servo_max_pos - servo_min_pos;

float angle = 0.0;
float speed = 0.001;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting postplotter ... Fasten your seatbelts! ");
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("");
  
  servo.attach(servo_pin);
  servo.write(90);
}


void loop() {
  angle += speed;
  pos = servo_center + range * .5 * sin(angle);
  servo.write(int(pos));
}
