#include "bsd.h"
#include <SoftwareSerial.h>

SoftwareSerial bsdSerial = SoftwareSerial(9, 10);

BSD bsd("bsd.cfg", bsdSerial);

void setup(){
  bsd.begin(9600);
  pinMode(13, OUTPUT);
}

void loop(){
  bsd.process();
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}
