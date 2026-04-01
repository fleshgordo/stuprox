/*
  Hall sensor calibration helper with EEPROM persistence.

  What this sketch does:
  - Reads hall sensor on A0.
  - Guides user through calibration over Serial Monitor.
  - Stores min/max values in EEPROM so they survive reset.

  Serial commands:
  - c : start calibration mode
  - s : stop calibration and save min/max
  - p : print current calibration
  - x : clear saved calibration
  - h : print help
*/

#include <EEPROM.h>

const int hallSensorPin = A0;

const int EEPROM_ADDR = 0;
const uint16_t EEPROM_MAGIC = 0xBEEF;

struct HallCalibration
{
  uint16_t magic;
  int minVal;
  int maxVal;
};

HallCalibration cal = {0, 1023, 0};
bool isCalibrating = false;
unsigned long lastPrintMs = 0;

void printHelp()
{
  Serial.println();
  Serial.println("Hall Sensor Calibration Commands:");
  Serial.println("  c = start calibration");
  Serial.println("  s = stop calibration + save to EEPROM");
  Serial.println("  p = print calibration");
  Serial.println("  x = clear saved calibration");
  Serial.println("  h = help");
  Serial.println();
}

void printCalibration()
{
  if (cal.magic != EEPROM_MAGIC)
  {
    Serial.println("Calibration: NOT SET");
    return;
  }

  Serial.print("Calibration min=");
  Serial.print(cal.minVal);
  Serial.print(" max=");
  Serial.println(cal.maxVal);
}

void loadCalibration()
{
  EEPROM.get(EEPROM_ADDR, cal);
  if (cal.magic != EEPROM_MAGIC || cal.minVal >= cal.maxVal)
  {
    cal.magic = 0;
    cal.minVal = 1023;
    cal.maxVal = 0;
    Serial.println("No valid saved calibration found.");
  }
  else
  {
    Serial.println("Loaded calibration from EEPROM.");
    printCalibration();
  }
}

void saveCalibration()
{
  cal.magic = EEPROM_MAGIC;
  EEPROM.put(EEPROM_ADDR, cal);
  Serial.println("Calibration saved to EEPROM.");
  printCalibration();
}

void clearCalibration()
{
  HallCalibration blank = {0, 1023, 0};
  EEPROM.put(EEPROM_ADDR, blank);
  cal = blank;
  Serial.println("Calibration cleared.");
}

void startCalibration()
{
  isCalibrating = true;
  cal.minVal = 1023;
  cal.maxVal = 0;

  Serial.println();
  Serial.println("Calibration started.");
  Serial.println("Move the magnet through your full range now.");
  Serial.println("When done, send 's' to save.");
}

void stopCalibrationAndSave()
{
  if (!isCalibrating)
  {
    Serial.println("Not in calibration mode.");
    return;
  }

  isCalibrating = false;

  if (cal.minVal >= cal.maxVal)
  {
    Serial.println("Calibration failed: min/max invalid. Try again.");
    return;
  }

  saveCalibration();
}

int normalizeToPercent(int rawValue)
{
  if (cal.magic != EEPROM_MAGIC || cal.maxVal <= cal.minVal)
  {
    return -1;
  }

  long mapped = map(rawValue, cal.minVal, cal.maxVal, 0, 100);
  if (mapped < 0)
  {
    mapped = 0;
  }
  if (mapped > 100)
  {
    mapped = 100;
  }
  return (int)mapped;
}

void handleSerial()
{
  while (Serial.available() > 0)
  {
    char cmd = (char)Serial.read();

    if (cmd == '\n' || cmd == '\r')
    {
      continue;
    }

    if (cmd == 'c')
    {
      startCalibration();
    }
    else if (cmd == 's')
    {
      stopCalibrationAndSave();
    }
    else if (cmd == 'p')
    {
      printCalibration();
    }
    else if (cmd == 'x')
    {
      clearCalibration();
    }
    else if (cmd == 'h')
    {
      printHelp();
    }
    else
    {
      Serial.print("Unknown command: ");
      Serial.println(cmd);
      printHelp();
    }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(hallSensorPin, INPUT);

  delay(250);
  Serial.println("Hall Sensor Calibration Tool");
  loadCalibration();
  printHelp();
}

void loop()
{
  handleSerial();

  int sensorValue = analogRead(hallSensorPin);

  if (isCalibrating)
  {
    if (sensorValue < cal.minVal)
    {
      cal.minVal = sensorValue;
    }
    if (sensorValue > cal.maxVal)
    {
      cal.maxVal = sensorValue;
    }
  }

  unsigned long now = millis();
  if (now - lastPrintMs >= 200)
  {
    lastPrintMs = now;

    Serial.print("raw=");
    Serial.print(sensorValue);

    int pct = normalizeToPercent(sensorValue);
    if (pct >= 0)
    {
      Serial.print(" pct=");
      Serial.print(pct);
    }

    if (isCalibrating)
    {
      Serial.print(" [cal min=");
      Serial.print(cal.minVal);
      Serial.print(" max=");
      Serial.print(cal.maxVal);
      Serial.print("]");
    }

    Serial.println();
  }
}
