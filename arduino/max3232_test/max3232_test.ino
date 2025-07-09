#include<SoftwareSerial.h>

const uint8_t rxPin=8;
const uint8_t txPin=9;
SoftwareSerial serial=SoftwareSerial(rxPin, txPin);

void setup() {
  Serial.begin(9600);
  serial.begin(9600);
  serial.println("MAX3232 RS232-to-TTL Test");
  Serial.println("MAX3232 RS232-to-TTL Test");		
  pinMode(13, OUTPUT);
}

void loop() {
  // Check if data is available from the RS232 device
  while (Serial.available()) {
    char data = Serial.read(); // Read data from the RS232 device
    serial.print(data); // Copy to monitor
  }
  serial.println();
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}

