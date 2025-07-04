#include "bsd.h"

BSD bsd;

void setup(){
  bsd.begin(9600);
  delay(2000);
  bsd.requestFile("bsd.cfg");
}

void loop(){
  bsd.process();
  bsd.print("done");
  delay(1000);
}
