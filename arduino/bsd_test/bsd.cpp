#include "bsd.h"
#include <string.h> // strcpy
#include <stdlib.h> // Include for strtol

void BSD::begin(const unsigned long baudrate) {
  if (baudrate) {
    serial_.begin(baudrate);
    monitor_.begin(baudrate);
  }
  else {
    serial_.begin(baudrate_);
    monitor_.begin(baudrate_);
  }
  // Flush input buffers
  delay(100);  // Small delay to ensure serial is ready
  // Clear input buffer multiple times
  for (int i = 0; i < 10; i++) {
    while (serial_.available()) {
      serial_.read();
    }
    delay(10);
  }
}

  
void BSD::process(){
  char c;
  // the status variable needs to be retained
  static uint8_t status=0; 
  while (serial_.available() > 0) {
    c = serial_.read();
    inputBuffer_[p_]=c;
    p_++;
    if (c=='$'){
      //We have seen a $. Note this.
      if (status & DOLLAR_SIGN_SEEN){
	// we see $ for the second time. Ignore what we have seen
	// before.
	p_=0; inputBuffer_[p_]=c; p_++;
	status &= ~DOLLAR_SIGN_SEEN;
      }
      else {
	status |= DOLLAR_SIGN_SEEN;
      }
    }
    if (c == '\n') {
      inputBuffer_[p_] = '\0'; //override \n by \0
      monitor_.print("inputBuffer_: ");
      monitor_.println(inputBuffer_);
      parse_buffer(&status);
      p_=0;
    }
    //if (status & CONFIGFILEREQUESTED){
    //}
  }
  /*
  if  (!(status & CONFIGFILEREQUESTED)){
    monitor_.print("<");
    monitor_.print(status);
    monitor_.println(">");
  }
  */
  if ((status & ACTIVE) && (!(status & CONFIGFILEREQUESTED))){
    status |= CONFIGFILEREQUESTED;
    requestConfigFile();
  }

  if ((status & CONFIGFILEREQUESTED) && (p_ == 63)){
    status |= DATABLOCKRECEIVED;
    parse_buffer(&status);
    status &= ~DATABLOCKRECEIVED;
    sendGoCommand();
  }
}


uint8_t BSD::get_identifier(char* identifierString,
			    uint8_t* pPayload,
			    const uint8_t size){
  bool status;
  uint8_t errorno=0;
  uint8_t p;
  
  status = (inputBuffer_[0]=='$');
  if (!status){
    errorno = ERROR_NO_DOLLAR_SIGN;
  }
  else if (size < IDENTIFIER_STRING_SIZE){
    errorno = ERROR_STRING_TOO_SHORT;
  }
  else {
    for  (p=1; p<IDENTIFIER_STRING_SIZE; p++){
      if( (inputBuffer_[p] == ',') || (inputBuffer_[p] == '*') ){
	break;
      }
    }
    if (p==size){
      errorno = ERROR_IDENTIFIER_NOT_FOUND;
    }
    else {
      //string is in inputBuffer_[1:p-1]
      monitor_.print(p);
      monitor_.print(" ");
      monitor_.println(inputBuffer_);

      strncpy(identifierString, &inputBuffer_[1], p-1);
      identifierString[p-1] = '\0';
      *pPayload = p;
    }
  }
  monitor_.println(inputBuffer_);
  monitor_.print("error: ");
  monitor_.println(errorno);
  return errorno;
}


uint8_t BSD::get_crc(char* crcString,
		     uint8_t* pPayload,
		     const uint8_t size){
  uint8_t errorno=0;
  uint8_t p;
  
  for(p=size; p>1; p--){
    if(inputBuffer_[p] == '*'){
      break;
    }
  }
  if (p<2){
    errorno = ERROR_CRC_NOT_FOUND;
  }
  else{
    p++;
    strncpy(crcString, &inputBuffer_[p], CRC_STRING_SIZE-1);
    crcString[CRC_STRING_SIZE-1] = '\0';
    *pPayload = p;
  }
  return errorno;
}

    
uint8_t BSD::compute_crc(uint8_t *crc, const char *buffer, const uint8_t size){
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
}
  

uint8_t BSD::parse_buffer(uint8_t *status){
  char identifierString[IDENTIFIER_STRING_SIZE];
  char crcString[CRC_STRING_SIZE];
  uint8_t crc, crc_payload;
  uint8_t p0, p1; // start and end positions of payload
  uint8_t errorno;

  uint8_t size = strlen(inputBuffer_);
  errorno = get_identifier(identifierString, &p0, size);
  errorno |= get_crc(crcString, &p1, size);
  errorno |= compute_crc(&crc_payload, inputBuffer_, size);
  if (errorno == ERROR_NO_ERROR){
    crc = strtol(crcString, NULL, 16);
    errorno |= (uint8_t)!(crc_payload==crc);
  }

  if (errorno == ERROR_NO_ERROR){
      if (strcmp(identifierString, "SD") == 0){
	*status|=ACTIVE;
	monitor_.println("Need to do SD.");
      }
      else if (strcmp(identifierString, "HI") == 0){
	*status|=ACTIVE;
      }
      else if (strcmp(identifierString, "BY") == 0){
	*status=INACTIVE;
      }
      else if (strcmp(identifierString, "FI") == 0){
	//snprintf(buffer, p1-p0, "%s", buffer+p0);
	monitor_.print("inputBuffer_ : ");
	monitor_.println(inputBuffer_);
	//readFileData();
      }
  }
  return errorno;
}

void BSD::requestConfigFile(){
  char command[22] = "$FR,";
  uint8_t i=0;
  uint8_t crc, size;

  size = strlen(configFilename_);
  for (i=0; i<size; ++i){
    command[i+4] = configFilename_[i];
  }
  // Add the trailing * 
  command[size+4] = '*';
  command[size+5] = '\0';
  compute_crc(&crc, command, size+5);
  sprintf(command + size + 5, "%02x\n\0", crc);
  delay(1000);
  //serial_.write(command);
  //serial_.write("$TXT,hello*16\n");
  serial_.write("$FR,bsd.cfg*01\n");
  serial_.flush();  // Wait until writing is complete.
  monitor_.println("File requested with command:");
  monitor_.println(command);
  // serial_.write("$GO*08\n");
  // serial_.flush();

}


void BSD::sendGoCommand(){
  serial_.write("$GO*08\n");
  serial_.flush();  // Wait until writing is complete.
}
