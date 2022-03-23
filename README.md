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

Short description for examples:

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
    

# Requirements
  - Arduino Uno + [Arduino IDE](https://www.arduino.cc/en/software/)
  - CNC shield, DRV8255 driver chips
  - [Stepperdriver Library](https://github.com/laurb9/StepperDriver) installed
