#include "bsd.h"
#include <SoftwareSerial.h>

const uint8_t rxPin=10, txPin=11;

SoftwareSerial bsdSerial = SoftwareSerial(rxPin, txPin);

BSD bsd("bsd.cfg", bsdSerial);

void setup(){
  pinMode(13, OUTPUT);
  bsd.begin(9600);
}

void loop(){
  bsd.process();
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(900);
}
