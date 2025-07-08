//#include<SoftwareSerial.h>

const uint8_t rxPin=10;
const uint8_t txPin=11;
//SoftwareSerial serial=SoftwareSerial(rxPin, txPin);

void setup() {
  Serial.begin(9600);
  Serial.println("MAX3232 RS232-to-TTL Test");
  pinMode(13, OUTPUT);
}

void loop() {
  // Check if data is available from the RS232 device
  while (Serial.available()) {
    char data = Serial.read(); // Read data from the RS232 device
    Serial.print("Received: ");
    Serial.println(data); // Print the received data to the serial Monitor
  }
  digitalWrite(13, HIGH);
  Serial.println("loop");
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}


