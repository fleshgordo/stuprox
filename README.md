# Studio Prototyping Extended

```
 ____ _____ _   _ ____  ____   ___   __  __
/ ___|_   _| | | |  _ \|  _ \ / _ \  \ \/ /
\___ \ | | | | | | |_) | |_) | | | |  \  /
 ___) || | | |_| |  __/|  _ <| |_| |  /  \
|____/ |_|  \___/|_|   |_| \_\\___/  /_/\_\                  
                                            by Tom Pawlofsky & Gordan Savicic
```

Starter files for Studio Prototyping Extended. Examples are based on the [BasicStepper Library](https://github.com/laurb9/StepperDriver) and adapted to our course module in Bachelor Digital Ideation HSLU Lucerne.

## Intro 

   - [Video Tutorials - Tube Switch Channel](https://tube.switch.ch/channels/Dcqw1ga3NL)
 
## Software requirements
 
   - [Arduino IDE ](https://www.arduino.cc/en/software)
   - [Library Stepperdriver for A4988](https://github.com/laurb9/StepperDriver)

## Examples

 - #### 01_BasicStepperDriver.ino
    Test two stepper motors attached to your shield. Make sure you have this running before continuing further
 - #### 02_BothSteppers.ino
    Showcase the use of SyncDriver and MultiDriver with both stepper motors attached
 - #### 03_BothSteppersNonBlocking.ino
    Showcase non-blocking function for both steppers
 - #### 04_ServoTest.ino
    Test sketch for single servo use
 - #### 05_ExternalInterrupt.ino
    Uses an external interrupt with an external switch to disable motors (useful for kill-switches)
 - #### 06_SerialCommunication.ino 
    Simple Serial communication protocol to send settings and positions to Arduino
    

# Hardware Requirements
  - Arduino UNO
  - CNC shield, DRV8255 driver chips

## Links
   - Pololu Infos for [driver A4988](https://www.pololu.com/product/1182) 
   - [Datasheet A4498 (PDF)](https://www.tme.eu/Document/25459777e672c305e474897eef284f74/POLOLU-2128.pdf)
