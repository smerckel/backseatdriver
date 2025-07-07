#include<SoftwareSerial.h>

SoftwareSerial serial=SoftwareSerial(2,3);


void setup() {
  Serial.begin(9600); // Initialize serial communication with the Arduino (TTL)
  Serial.println("MAX3232 RS232-to-TTL Test");
  serial.begin(9600);
  pinMode(13, OUTPUT);

}

void loop() {
  // Check if data is available from the RS232 device
  if (serial.available()) {
    char data = Serial.read(); // Read data from the RS232 device
    Serial.print("Received: ");
    Serial.println(data); // Print the received data to the Serial Monitor

    // Echo the data back to the RS232 device
    serial.write(data);
  }
  digitalWrite(13, HIGH);
  serial.write("hello\r\n");
  Serial.write("hello\r\n");
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}


