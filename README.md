# Studio Prototyping Extended

```
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\                  
                                            by Tom Pawlofsky & Gordan Savicic
```

Starter files for Studio Prototyping Extended. Examples are based on the [BasicStepper Library](https://github.com/laurb9/StepperDriver) and [AccelStepper library](https://www.airspayce.com/mikem/arduino/AccelStepper/) adapted to our course module in Bachelor Digital Ideation HSLU Lucerne.

## Intro 

   - [Video Tutorials for Motor kit Tube Switch Channel](https://tube.switch.ch/channels/Dcqw1ga3NL)
   - [Slides Stepper Motor (download PDF in Ilias)](https://elearning.hslu.ch/ilias/goto.php?target=file_5832681_download&client_id=hslu)
 
## Software requirements
 
   - [Arduino IDE ](https://www.arduino.cc/en/software)
   - [Library Stepperdriver for A4988](https://github.com/laurb9/StepperDriver)
   - [Library AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/index.html)

 ## Checklist before Take-Off
 
   - Jumper gesetzt (Mitte 1/4step) ? 
   - Ref Voltage auf 0.6V eingestellt ?
   - 12V ans CNC Shield (Polarität beachten) ?
   - Servo: rot an +6V, schwarz GND am Shield, Signal Pin an SpinEn (D12)
   - Stepper angeschlossen ? 
   - Arduino USB an Computer ?
   - Arduino IDE + Library installiert ?
   - Stellring Schraube am Schrittmotor angezogen?
   - Netzteil 12V eingeschalten?
  
## Examples

 - #### [00_StepperIntro.ino](00_StepperIntro/)
    Move a stepper motor. Basic example
 - #### [01_ServoIntro.ino](01_ServoIntro/)
    Move a servo motor. Basic example
 - #### [02_ServoAndStepperBlocking.ino](02_ServoAndStepperBlocking/)
    Combine both intros to run a servo and stepper in blocking mode
 - #### [03_ServoAndStepperNonBlocking.ino](./03_ServoAndStepperNonBlocking/)
    Combine both intros to run a servo and stepper in non-blocking mode
 - #### [04_ServoAndStepperNonBlockingDrawing.ino](./04_ServoAndStepperNonBlockingDrawing/)
    Draw something with servo and stepper attached
 - #### [05_ExternalInterrupt.ino](./05_ExternalInterrupt/)
    Uses an external interrupt with an external switch to disable motors (useful for kill-switches)
 - #### [06_SerialCommunication.ino](./06_SerialCommunication/)
    Simple Serial communication protocol to send settings and positions to Arduino from Serial monitor, p5js or other software interfaces
- #### [07_GRBL](./07_GRBL/)
    Custom firmware for the Arduino that turns it into a Gcode motion control for CNC machines. You can upload and control the machine through a Desktop controller such as [cncjs](https://cnc.js.org/)
- #### [08_Servobot.ino](./08_Servobot/)
    Using two servo arms to create a mini-rapid-prototyped-drawmatic-postographo-plotter. It communicates with the computer through serial communication, directly interfacing with a p5js sketch that uses inverse kinematics to calculate angles.
- #### [09_AccelStepper.ino](./09_AccelStepper/)
    AccelStepper library has much better support for acceleration and speed settings than BasicStepper. Read the [documentation](https://www.airspayce.com/mikem/arduino/AccelStepper/) and the [missing guide](https://hackaday.io/project/183279-accelstepper-the-missing-manual/details) for detailed explanations.

# Hardware Requirements
  - Arduino UNO
  - CNC shield, A4988 driver chips

## Links
   - Pololu Infos for [driver A4988](https://www.pololu.com/product/1182) 
   - [Datasheet A4498 (PDF)](https://www.tme.eu/Document/25459777e672c305e474897eef284f74/POLOLU-2128.pdf)
   - [Video Tutorials for Motor kit Tube Switch Channel](https://tube.switch.ch/channels/Dcqw1ga3NL)
