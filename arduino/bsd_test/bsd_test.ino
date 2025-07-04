#include "bsd.h"
#include <SoftwareSerial.h>

SoftwareSerial monitor = SoftwareSerial(9, 10);

BSD bsd(monitor);

void setup(){
  monitor.begin(9600);
  monitor.println("bsd_test started");
  bsd.begin(9600);
  monitor.println("exit setup");
}

void loop(){
  //bsd.requestFile("bsd.cfg");
  bsd.process();
  delay(1000);
}
