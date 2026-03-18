/*
  01_BlinkNoDelaySerial.ino
  - Blink ohne delay()
  - Serielle Ausgabe bei jedem Umschalten

 ⚠️ Short to variables
 📐 Why Different Types? - Memory & Range!
    Size matters on Arduino

    int // -32,768 to 32,767 (2 bytes)
    unsigned long // 0 to 4,294,967,295 (4 bytes)
    bool // true/false (1 byte)

  */

const int LED_PIN = LED_BUILTIN;             // Pin numbers: 0-13 (int is overkill but standard)
const unsigned long BLINK_INTERVAL_MS = 500; // ⚠️ MUST be unsigned long to match millis()

unsigned long previousMillis = 0; // millis() returns unsigned long (up to 49 days!)
bool ledState = false;            // Only ON/OFF - so why waste bytes ;) bool uses 1 byte instead of int (2 bytes)?

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.begin(115200);
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
