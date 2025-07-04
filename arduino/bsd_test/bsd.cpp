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


uint8_t BSD::get_identifier(char* identifierString,
			    uint8_t* pPayload,
			    const char* buffer,
			    const uint8_t size){
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
      *pPayload = p;
    }
  }
  return errorno;
}


uint8_t BSD::get_crc(char* crcString,
		     uint8_t* pPayload,
		     const char* buffer,
		     const uint8_t size){
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
    *pPayload = p;
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

void BSD::readFileData(const char* encodedString){
  uint8_t encodedStringLength;

  encodedStringLength = strlen(encodedString);
  
  uint8_t decodedLength = Base64.decodedLength(encodedString,
					       encodedStringLength);
  char decodedString[decodedLength + 1];
  Base64.decode(decodedString,
		encodedString,
		encodedStringLength);
  print(decodedString);
}
  

bool BSD::parse_buffer(const char* buffer, const uint8_t size){
  char identifierString[IDENTIFIER_STRING_SIZE];
  char crcString[CRC_STRING_SIZE];
  uint8_t crc, crc_payload;
  uint8_t p0, p1; // start and end positions of payload
  uint8_t errorno;

  errorno = get_identifier(identifierString, &p0, buffer, size);
  errorno |= get_crc(crcString, &p1, buffer, size);
  errorno |= compute_crc(&crc_payload, buffer, size);

  if (errorno == ERROR_NO_ERROR){
      crc = strtol(crcString, NULL, 16);
      if (strcmp(identifierString, "SD") == 0){}
      else if (strcmp(identifierString, "HI") == 0){}
      else if (strcmp(identifierString, "FI") == 0){
	snprintf(buffer, p1-p0, "%s", buffer+p0);
	readFileData(buffer);
      }
  }
  else{
    crc=0xff;
  }
  return (crc_payload == crc);
}

void BSD::requestFile(const char* filename){
  char command[22] = "$FR,";
  uint8_t i=0;
  uint8_t crc, size;

  size = strlen(filename);
  for (i=0; i<size; ++i){
    command[i+4] = filename[i];
  }
  // Add the trailing * 
  command[size+4] = '*';
  command[size+5] = '\0';
  compute_crc(&crc, command, size+5);
  sprintf(command + size + 5, "%02x\r\n\0", crc);
  serial_.write(command);
}

void BSD::print(const char* s) {
  char command[70] = "$TXT, (debug)";
  uint8_t i=0;
  uint8_t crc, size;

  size = strlen(s);
  for (i=0; i<size; ++i){
    command[i+13] = s[i];
  }
  // Add the trailing * 
  command[size+13] = '*';
  command[size+14] = '\0';
  compute_crc(&crc, command, size+14);
  sprintf(command + size + 14, "%02x\r\n\0", crc);
  serial_.write(command);
}

void BSD::print(const uint8_t i) {
  char command[70] = "$TXT, (debug)";
  uint8_t crc, size;

  sprintf(command+13, "%d", i);
  size = strlen(command);
  // Add the trailing * 
  command[size] = '*';
  command[size+1] = '\0';
  compute_crc(&crc, command, size+1);
  sprintf(command + size + 1, "%02x\r\n\0", crc);
  serial_.write(command);
}
