#include "bsd.h"
#include <string.h> // strcpy
#include <stdlib.h> // Include for strtol

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


uint8_t BSD::get_identifier(char* identifierString, const char* buffer, const uint8_t size){
  bool status;
  uint8_t errorno=0;
  uint8_t p;
  
  status = (buffer[0]=='$');
  if (!status){
    errorno = ERROR_NO_DOLLAR_SIGN;
  }
  else if (size < IDENTIFIER_STRING_SIZE){
    errorno = ERROR_STRING_TOO_SHORT;
  }
  else {
    for  (p=1; p<IDENTIFIER_STRING_SIZE; p++){
      if( (buffer[p] == ',') || (buffer[p] == '*') ){
	break;
      }
    }
    if (p==size){
      errorno = ERROR_IDENTIFIER_NOT_FOUND;
    }
    else {
      //string is in buffer[1:p-1]
      strncpy(identifierString, &buffer[1], p-1);
      identifierString[p-1] = '\0';
    }
  }
  return errorno;
}


uint8_t BSD::get_crc(char* crcString, const char* buffer, const uint8_t size){
  uint8_t errorno=0;
  uint8_t p;
  
  for(p=size; p>1; p--){
    if(buffer[p] == '*'){
      break;
    }
  }
  if (p<2){
    errorno = ERROR_CRC_NOT_FOUND;
  }
  else{
    p++;
    strncpy(crcString, &buffer[p], CRC_STRING_SIZE-1);
    crcString[CRC_STRING_SIZE-1] = '\0';
  }
  return errorno;
}

    
uint8_t BSD::compute_crc(uint8_t *crc, const char* buffer, const uint8_t size){
  uint8_t status=0;
  uint8_t errorno = ERROR_NO_ERROR;
  *crc=0;
  for(uint8_t p=0; p<size; ++p){
    if (buffer[p]=='*'){
      status++;
      break;
    }
    if (status==1){
      *crc ^= buffer[p];
    }
    if (buffer[p]=='$'){
      status=1;
    }
  }
  if (status != 2)
    errorno = ERROR_PAYLOAD_NOT_FOUND;
  return errorno;
}


bool BSD::parse_buffer(const char* buffer, const uint8_t size){
  char identifierString[IDENTIFIER_STRING_SIZE];
  char crcString[CRC_STRING_SIZE];
  uint8_t crc, crc_payload;
  uint8_t errorno;

  errorno = get_identifier(identifierString, buffer, size);
  errorno |= get_crc(crcString, buffer, size);
  errorno |= compute_crc(&crc_payload, buffer, size);

  if (errorno == ERROR_NO_ERROR){
      crc = strtol(crcString, NULL, 16);
      if (strcmp(identifierString, "SD") == 0){}
      else if (strcmp(identifierString, "HI") == 0){}
  }
  else{
    crc=0xff;
  }
  serial_.println(buffer);
  serial_.println(errorno);
  serial_.println(identifierString);

  


  serial_.print(crcString);
  serial_.print(" ");
  serial_.print(crc_payload);
  serial_.print(" ");
  serial_.println(crc);
  serial_.println();

  return (crc_payload == crc);
}

void BSD::print() {
   serial_.println("Hello my world");
   serial_.println(baudrate_);
}
