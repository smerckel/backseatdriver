#include "bsd.h"


void BSD::begin(const unsigned long baudrate) {
  if (baudrate) {
      serial_.begin(baudrate);
  }
  else {
    serial_.begin(baudrate_);
  }
}

void BSD::process(){
  char c;
  int p=0;
  char buffer[BUFFERSIZE];
  
  while (serial_.available() > 0) {
    c = serial_.read();
    buffer[p++]=c;
    if (c == '\n') {
      buffer[p] = '\0';
      parse_buffer(buffer, p);
      p=0;
    }
  }
}


uint8_t get_identifier(char* identifierString, const char* buffer, const uint8_t size){
  bool status;
  uint8_t errorno=0;
  
  status = (buffer[0]=='$');
  if (!status){
    errorno=ERROR_NO_DOLLAR_SIGN;
  }
  else {
    for  (uint8_t p=0; p<IDENTIFIER_STRING_SIZE; p++){
      
    }
  }
  return errorno;
}

void BSD::parse_buffer(char* buffer, uint8_t size){
    serial_.print(buffer);
}

void BSD::print() {
   serial_.println("Hello my world");
   serial_.println(baudrate_);
}
