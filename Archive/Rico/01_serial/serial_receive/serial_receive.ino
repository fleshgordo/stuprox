// Arduino code to receive grayscale values via serial port

const int pinOut = 2;

const int chunkSize = 8;  // Number of values to read in each chunk
int values[chunkSize];

void setup() {
  Serial.begin(9600);
  pinMode(pinOut, OUTPUT);
}

void loop() {
  if (Serial.available()) {
    // Read incoming chunk of grayscale values
    for (int i = 0; i < chunkSize; i++) {
      values[i] = Serial.parseInt();
    }

    // Do something with the received values
    for (int i = 0; i < chunkSize; i++) {
      if (values[i] == 0) {
        digitalWrite(pinOut, LOW);
      } else {
        digitalWrite(pinOut, HIGH);
      }
      Serial.print(values[i]);
      Serial.print(',');
    }
    Serial.println();  // Print newline to indicate end of chunk
  }
  //Serial.println("---");
}