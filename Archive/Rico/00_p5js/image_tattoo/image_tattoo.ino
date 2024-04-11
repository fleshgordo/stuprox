
const byte grayscaleValues[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

const int outputPin = 13;  // Change this to your desired output pin

void setup() {
  pinMode(outputPin, OUTPUT);
  for (int i = 0; i < sizeof(grayscaleValues); i++) {
    if (grayscaleValues[i] == 0) {
      digitalWrite(outputPin, LOW);
    } else {
      digitalWrite(outputPin, HIGH);
    }
    delay(20);  // Adjust delay as needed
  }
  Serial.println("done");
}

void loop() {
}