#ifndef __BSD_H__
#define __BSD_H__

#include <Arduino.h>

const uint8_t ERROR_NO_ERROR=0;
const uint8_t ERROR_NO_DOLLAR_SIGN=1;

const uint8_t BUFFERSIZE = 128;
const uint8_t IDENTIFIER_STRING_SIZE = 4; // TXT + \0

class BSD{
public:
  //Constructor using standard Serial input
  BSD(HardwareSerial& serial=Serial, const unsigned long baudrate=9600)
    : serial_{serial}, baudrate_{baudrate} {
  }

  void begin(const unsigned long baudrate=0);
  void process();
  void parse_buffer(char* buffer, const uint8_t size);
  uint8_t get_identifier(char* identifierString, const char* buffer, const uint8_t size);
  
  void print();
  
private:
  HardwareSerial& serial_;
  unsigned long baudrate_;
};

#endif
