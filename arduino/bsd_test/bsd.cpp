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
  // // Flush input buffers
  // delay(100);  // Small delay to ensure serial is ready
  // // Clear input buffer multiple times
  // for (int i = 0; i < 10; i++) {
  //   while (serial_.available()) {
  //     serial_.read();
  //   }
  //   delay(10);
  // }
}


void BSD::processInputBuffer(){
  /* This method processes the inputBuffer_. Sometimes this buffer
     contains two sentences. This method splits them and feeds
     each sentence to the parseBuffer() method.
     It turns out that each sentence ends with \r\n. The
     buffer sent to parseBuffer is properly terminated, but
     has no \r\n characters anymore.
  */
  char buffer[INPUTBUFFERSIZE];
  uint8_t p=0, inputBufferSize;

  inputBufferSize = strlen(inputBuffer_);
  for(uint8_t k=0; k<inputBufferSize; k++){
    buffer[p] = inputBuffer_[k];
    if (inputBuffer_[k]=='\n'){
      buffer[p-1]='\0'; //overwrites the \r
      monitor_.print("buffer: ");
      monitor_.println(buffer);
      parseBuffer(buffer);
      p=0;
    }
    else
      p++;
  }
}



void BSD::process(){
  char c;
  // the status variable needs to be retained
  while (serial_.available() > 0) {
    c = serial_.read();
    inputBuffer_[p_]=c;
    p_++;
  }

  if (p_ > 0){
    inputBuffer_[p_]='\0';
    // just for monitoring wha
    // monitor_.print(p_);
    // monitor_.print(" : ");
    // monitor_.println(inputBuffer_);
    // 
    processInputBuffer();
  }

  p_ = 0;
}




uint8_t BSD::getIdentifier(char* identifierString,
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
      strncpy(identifierString, &buffer[1], p-1);
      identifierString[p-1] = '\0';
      *pPayload = p+1; //  buffer[p]==',' payload starts 1 after.
    }
  }
  return errorno;
}


uint8_t BSD::getCrc(char* crcString,
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
    *pPayload = p-1; // p is at start of crc code. Points to 1 byte after payload
  }
  return errorno;
}

    
uint8_t BSD::computeCrc(uint8_t *crc, const char *buffer, const uint8_t size){
  uint8_t status=0;
  uint8_t errorno = NO_ERROR;
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

void BSD::decodeBase64(char* decodedString, const char* encodedString){
  uint8_t encodedStringLength = strlen(encodedString);
  uint8_t decodedLength = Base64.decodedLength(encodedString,
   					       encodedStringLength);
  Base64.decode(decodedString,
		encodedString,
		encodedStringLength);
  // From some reason, the first three characters are always the same and nonsense.
  for (uint8_t k=3; k<decodedLength; k++){
    decodedString[k-3] = decodedString[k];
  }
}

void BSD::readFileData(const char* encodedString){
  static uint8_t status=EXPECTINGKEYWORD;
  static char keyword[MAXKEYWORDSIZE];
  static char value[MAXVALUESIZE];
  static uint8_t pKeyword=0;
  static uint8_t pValue=0;
  
  char decodedString[DECODEDSTRINGSIZE];
  decodeBase64(decodedString, encodedString);
  uint8_t size = strlen(decodedString);
  
  for (uint8_t p=0; p<size; p++){
    if (decodedString[p]=='#')
      status |= ISCOMMENT;
    if (status & ISCOMMENT){
      if (decodedString[p]=='\n')
	status &= ~ISCOMMENT; // clear flag
      continue; // Ignores all characters on a comment line
    }
    else if (decodedString[p]=='\r')
      continue; // ignore any \r that may be
    // From here we have something of interest.
    if (decodedString[p]=='='){
      status &= ~ EXPECTINGKEYWORD;
      status |= EXPECTINGVALUE;
      continue;
    }
    else if (decodedString[p]=='\n'){
      status = PROCESSKEYVALUEPAIR; // clears all other flags too.
    }
    //
    if ((int)decodedString[p]>48)
      monitor_.print(decodedString[p]);
    else
      monitor_.print("*");
    monitor_.print(" : ");
    monitor_.print((int)decodedString[p]);
    monitor_.print(" : ");
    monitor_.println(status);
    
    if (status & EXPECTINGKEYWORD){
      keyword[pKeyword]=decodedString[p];
      pKeyword++;
    }
    else if (status & EXPECTINGVALUE){
      value[pValue]=decodedString[p];
      pValue++;
    }
    if (status & PROCESSKEYVALUEPAIR){
      keyword[++pKeyword]='\0';
      value[++pValue]='\0';
      monitor_.print("processing: ");
      monitor_.print(keyword);
      monitor_.print("=");
      monitor_.println(value);

      if (strcmp(keyword, "dmin")==0)
	dmin_ = atof(value);
      else if (strcmp(keyword, "dmax")==0)
	dmax_ = atof(value);
      else if (strcmp(keyword, "threshold")==0)
	threshold_ = atof(value);
      else if (strcmp(keyword, "n_profiles")==0)
	nprofiles_ = atoi(value);

      monitor_.print(dmin_);
      monitor_.print(":");
      monitor_.print(dmax_);
      monitor_.print(":");
      monitor_.print(threshold_);
      monitor_.print(":");
      monitor_.println(nprofiles_);
      
      // do processing

      pKeyword=0;
      pValue=0;
      status = EXPECTINGKEYWORD;
    }
  }
}

void BSD::sendSWmessage(uint8_t index, int value){
  char buffer[INPUTBUFFERSIZE];
  uint8_t crc;
  
  strncpy(buffer,"$SW,", 4);
  sprintf(&(buffer[4]), "%d,%d\0", index, value);
  computeCrc(&crc, buffer, strlen(buffer));
  sprintf(&(buffer[4]), "%d,%d*%02x\n\0", index, value, crc);
  monitor_.print("SW message: ");
  monitor_.println(buffer);
  serial_.write(buffer);
  
}

void BSD::sendSWmessage(uint8_t index, float value){
  char buffer[INPUTBUFFERSIZE];
  char floatString[MAXVALUESIZE];
  uint8_t crc;
  
  strncpy(buffer,"$SW,", 4);
  dtostrf(value, 1, 3, floatString);
  sprintf(&(buffer[4]), "%d:%s\0", index, floatString);
  computeCrc(&crc, buffer, strlen(buffer));
  sprintf(&(buffer[4]), "%d:%s*%02x\n\0", index, floatString, crc);
  monitor_.print("SW message: ");
  monitor_.println(buffer);
  serial_.write(buffer);
}

void BSD::updateMissionParameters(){
  static uint8_t status=0;
  monitor_.println("entering Update mission parameters ...");
  if ((sci_water_pressure_< -10) || (sci_water_temp_ < -10)){
    monitor_.println("exiting Update mission parameters early...");
    return;
  }
  /* Note: the sendSWmessage expects an int or float, not double. If
     you supply just a float, then it is interpreted as a double. Type
     case it first.

     Example:
     
     sendSWmessage(0,  (float) 25.0);
  */
  monitor_.println("Update mission parameters...");
  monitor_.print("Status : ");
  monitor_.println(status);
  if (status==0){
    sendSWmessage(0,  (float) 25.0);
    monitor_.println("Called sendSWmessage.");
    status=1;
  }
  
//   if(sci_water_pressure_*10>dmin_){
//     sendSWmessage(0, (float) 25.0);
//   }
}


void BSD::parsePayloadSD(const char* buffer, uint8_t p0, uint8_t p1){
  uint8_t status=0, pValue=0;
  uint8_t parameterIndex;
  char valueString[MAXVALUESIZE];
  
  for(uint8_t p=p0; p<=p1; p++)
    {
      if (buffer[p]==':'){
	status=1;
	continue;
      }
      if ((buffer[p]==',') || (buffer[p]=='*')){
	valueString[pValue]='\0';
	/*Note the parameter index is counted THROUGHOUT the exctl file!!!

	  This means that any ouput parameters in mp and os section also count.
	  Just so you know...
	*/
	switch (parameterIndex){
	case 1:
	  sci_water_pressure_ = atof(valueString);
	  break;
	case 2:
	  sci_water_temp_ = atof(valueString);
	  break;
	}
	pValue=0;
	status=0;
      }
      else if (status==0){
	parameterIndex = (uint8_t) buffer[p] - 48;
      }
      else if (status==1){
	valueString[pValue++]=buffer[p];
      }
    }
  monitor_.println(sci_water_temp_);
  monitor_.println(sci_water_pressure_);
}
	

uint8_t BSD::parseBuffer(const char *buffer){
  char identifierString[IDENTIFIER_STRING_SIZE];
  char crcString[CRC_STRING_SIZE];
  uint8_t crc=0, crc_payload;
  uint8_t p0, p1; // start and end positions of payload
  uint8_t errorno;

  uint8_t size = strlen(buffer);
  
  errorno = getIdentifier(identifierString, &p0, buffer, size);
  errorno |= getCrc(crcString, &p1, buffer, size);
  errorno |= computeCrc(&crc_payload, buffer, size);
  if (errorno == NO_ERROR){
    crc = strtol(crcString, NULL, 16);
    errorno |= (uint8_t)!(crc_payload==crc);
  }
  if (errorno == NO_ERROR){
    if (strcmp(identifierString, "SD") == 0){
      status_ |= ACTIVE;
      status_ |= DO_SD; 
    }
    else if (strcmp(identifierString, "HI") == 0){
      status_ |= ACTIVE;
    }
    else if (strcmp(identifierString, "BY") == 0){
      status_ = INACTIVE; // Clears all flags
    }
    else if (strcmp(identifierString, "FI") == 0){
      if(strcmp(buffer, "$FI*0f")==0)
	status_ |= FILETRANSFERCOMPLETE;
      else
	status_ |= DATABLOCKRECEIVED;
    }
  }
  else if ((errorno > 0) && (status_ & CONFIGFILEREQUESTED)){
    /* Something went wrong during file transfer. Cancel current
       transaction and start again.
    */
    status_ &= ~CONFIGFILEREQUESTED;
    status_ &= ~DATABLOCKRECEIVED;
    return errorno;
  }
  monitor_.print("Error no : ");monitor_.println(errorno);
  monitor_.println(identifierString);
  monitor_.print("Status : ");monitor_.println(status_);
  monitor_.println("----");

  if ((status_ & ACTIVE) && (status_ & DO_SD)){
    monitor_.println("about to do parsepayload");
    parsePayloadSD(buffer, p0, p1);
    monitor_.println("about to do update");
    updateMissionParameters();
    monitor_.println("done with update");
  }
    
  if ((status_ & ACTIVE) && (!(status_ & CONFIGFILEREQUESTED)) ){
    status_ |= CONFIGFILEREQUESTED;
    //requestConfigFile();
  }

  if ((status_ & ACTIVE) && ((status_ & DATABLOCKRECEIVED)) ){
    status_ &= ~DATABLOCKRECEIVED; // Clear this flag
    readFileData(buffer);
    sendGoCommand();
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
  computeCrc(&crc, command, size+5);
  sprintf(command + size + 5, "%02x\n\0", crc);
  serial_.write(command);
  serial_.flush();  // Wait until writing is complete.
  monitor_.println("File requested with command:");
  monitor_.println(command);
}


void BSD::sendGoCommand(){
  delay(1000);
  serial_.write("$GO*08\n");
  serial_.flush();  // Wait until writing is complete.
  monitor_.println("Sent command: $GO*08\n");
}
