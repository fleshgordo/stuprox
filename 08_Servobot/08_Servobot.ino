/*
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\
                                            by Tom Pawlofsky & Gordan Savicic

*/

#include <Servo.h>

Servo myservo1;
Servo myservo2; // create servo object to control a servo

const int SERVO_PIN1 = 9;
const int SERVO_PIN2 = 10;

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;      // toggle new message

void setup()
{
  myservo1.attach(SERVO_PIN1); // attaches the servo on pin 9 to the servo object
  myservo2.attach(SERVO_PIN2);

  Serial.begin(9600);
  Serial.println("Booting postplotter ... Fasten your seatbelts! ");
  Serial.println("");
  Serial.println(" ____ _____ _   _ ____  ____   ___   __  __");
  Serial.println("/ ___|_   _| | | |  _ \\|  _ \\ / _ \\  \\ \\/ /");
  Serial.println("\\___ \\ | | | | | | |_) | |_) | | | |  \\  /");
  Serial.println(" ___) || | | |_| |  __/|  _ <| |_| |  /  \\");
  Serial.println("|____/ |_|  \\___/|_|   |_| \\_\\\\___/  /_/\\_\\");
  Serial.println("");
  Serial.println("Press ? for help");
}

void loop()
{
  receiveData();
  if (newData == true)
  {
    handleData();
    newData = false;
  }
}

void receiveData()
{
  static byte ndx = 0;
  char endMarker = '\n';
  char receivingChar;
  while (Serial.available() > 0 && newData == false)
  {
    receivingChar = Serial.read();
    if (receivingChar != endMarker)
    {
      receivedChars[ndx] = receivingChar;
      ndx++;
      if (ndx >= numChars)
      {
        ndx = numChars - 1;
      }
    }
    else
    {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void handleData()
{
  Serial.print("DEBUG: This came in ... ");
  Serial.println(receivedChars);

  if (receivedChars[0] == 'X')
  {
    int pos_1;
    int pos_2;
    sscanf(receivedChars, "X%d,Y%d", &pos_1, &pos_2);
    char buffer[40];
    Serial.print("*** *** Moving servo angle 1: X:");
    Serial.print(pos_1);
    Serial.print(" Y:");
    Serial.println(pos_2);

    myservo1.write(pos_1);
    myservo2.write(pos_2);
    Serial.println("*** DONE ");
  }

  else if (receivedChars[0] == '?')
  {
    Serial.println("------------------------------------------------------------");
    Serial.println("HELP ");
    Serial.println("------------------------------------------------------------");
    Serial.println("Use following commands to interact with servobot: ");
    Serial.println("X10,Y40 - rotate servo 1 to angle 10, rotate servo 2 to angle 40");
  }
}
