/*
  01_BlinkNoDelaySerial.ino
  - Blink ohne delay()
  - Serielle Ausgabe bei jedem Umschalten
*/

const uint8_t LED_PIN = LED_BUILTIN;
const unsigned long BLINK_INTERVAL_MS = 500;

unsigned long previousMillis = 0;
bool ledState = false;

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }
    Serial.println("");
    Serial.println(" ____ _____ _   _ ______  __");
    Serial.println("/ ___|_   _| | | |  _ \\ \\/ /");
    Serial.println("\\___ \\ | | | | | | |_) \\  /");
    Serial.println(" ___) || | | |_| |  __//  \\");
    Serial.println("|____/ |_|  \\___/|_|  /_/\\_\\");
    Serial.println("");
    Serial.println("KICKSTART 01: Blink without delay + Serial");
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= BLINK_INTERVAL_MS)
    {
        previousMillis = currentMillis;
        ledState = !ledState;

        digitalWrite(LED_PIN, ledState ? HIGH : LOW);

        Serial.print("LED: ");
        Serial.print(ledState ? "ON" : "OFF");
        Serial.print(" | millis: ");
        Serial.println(currentMillis);
    }
}
