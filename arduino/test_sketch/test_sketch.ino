#include "Base64.h"

#include <SoftwareSerial.h>

SoftwareSerial monitor = SoftwareSerial(9,10);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\n-------------");
  monitor.begin(9600);
  monitor.println("HW");
  //char s[128] = "dGhyZXNob2xkOjEuMjM0Cm5fcHJvZmlsZXM6Mw==";
  char s[128] = "I3NvbWUgY29tbWVudHMKZG1pbjoyNjUKZG1heDozNjUKdGhyZXNob2xkOjEuMjM0Cm5fcHJvZmlsZXM6Mw==";
  uint8_t sLength = strlen(s);
  uint8_t dLength = Base64.decodedLength(s, sLength);

  //Serial.println(s);
  //Serial.println(sLength);
  //Serial.println(dLength);

  char d[dLength+1];
  Base64.decode(d, s, sLength);
  //Serial.println(d);
  const uint8_t KEYWORDSIZE=8;
  char c, keyword[KEYWORDSIZE+1], valueString[KEYWORDSIZE+1];
  /* strings are of the form  text:value\r\n per line.*/
  uint8_t status=0; // 0 line with values 1 line with comments
  uint8_t p=0;
  for(uint8_t i=0; i< dLength; i++){
    if (d[i]=='#'){
      /* line with comments. Read until \n*/
      status=1;
    }
    if (status==1){
      /* we are reading comments.*/
      if (d[i]=='\n'){
        status=0;
        p=i+1; // position on next line.
      }
    }
    else {
      /* we are reading a valid line*/
      if (d[i]==':'){
        strncpy(keyword, &d[p], min(i-p,KEYWORDSIZE));
        keyword[min(i-p,KEYWORDSIZE)]='\0';
        p=i+1;
        Serial.print(keyword);
        Serial.print("=");
      }
      if (d[i]=='\n'){
        strncpy(valueString, &d[p], min(i-p,KEYWORDSIZE)); 
        valueString[min(i-p,KEYWORDSIZE)]='\0';
        p=i+1;
        Serial.println(valueString);

      }
    }
    //Serial.print(i); Serial.print(" "); Serial.println(dLength);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  uint8_t i,j;
  i=1; j=1;
  Serial.println((uint8_t)!(i==j));
}
