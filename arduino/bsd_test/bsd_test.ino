#include "bsd.h"

BSD bsd;

void setup(){
bsd.begin(9600);  
}

void loop(){
  bsd.process();
  // bsd.print();
  delay(1000);

}