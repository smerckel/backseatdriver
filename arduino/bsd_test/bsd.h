#ifndef __BSD_H__
#define __BSD_H__

#include <Arduino.h>
#include "Base64.h"
#include <SoftwareSerial.h>

const uint8_t ERROR_NO_ERROR = 0;
const uint8_t ERROR_NO_DOLLAR_SIGN = 1 << 0;
const uint8_t ERROR_STRING_TOO_SHORT = 1 << 1;
const uint8_t ERROR_IDENTIFIER_NOT_FOUND = 1 << 2;
const uint8_t ERROR_CRC_NOT_FOUND = 1 << 3;
const uint8_t ERROR_PAYLOAD_NOT_FOUND = 1 << 4;

// Status bits
const uint8_t INACTIVE = 0;
const uint8_t ACTIVE = 1 << 0;
const uint8_t CONFIGFILEREQUESTED = 1 << 1;
const uint8_t DOLLAR_SIGN_SEEN = 1 << 2;
const uint8_t DATABLOCKRECEIVED = 1 << 3;

const uint8_t INPUTBUFFERSIZE = 127;
const uint8_t IDENTIFIER_STRING_SIZE = 4; // TXT + \0
const uint8_t CRC_STRING_SIZE = 3; // 3e + \0

class BSD{
public:
  //Constructor using standard Serial input

  BSD(const char* configFilename,
      SoftwareSerial& monitor,
      HardwareSerial& serial=Serial,
      const unsigned long baudrate=9600)
    : configFilename_{configFilename}, monitor_{monitor}, serial_{serial}, baudrate_{baudrate}, p_{0}{
  }

  void begin(const unsigned long baudrate=0);
  
  void process();
  
  uint8_t parse_buffer(uint8_t *status);
  
  uint8_t get_identifier(char* identifierString,
			 uint8_t *pPayload,
			 const uint8_t size);
  
  uint8_t get_crc(char* crcString,
		  uint8_t *pPayload, 
		  const uint8_t size);
  
  uint8_t compute_crc(uint8_t *crc,
		      const char* buffer,
		      const uint8_t size);
  
  void requestConfigFile();

  void sendGoCommand();

  void readFileData(const char* encodedString);
  
private:
  HardwareSerial& serial_;
  SoftwareSerial& monitor_;
  const unsigned long baudrate_;
  const char* configFilename_;

  // The input buffer and its pointer.
  char inputBuffer_[INPUTBUFFERSIZE];
  uint8_t p_;
  
};


#endif
