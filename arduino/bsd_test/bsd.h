#ifndef __BSD_H__
#define __BSD_H__

#include <Arduino.h>

const uint8_t ERROR_NO_ERROR = 0;
const uint8_t ERROR_NO_DOLLAR_SIGN = 1 << 1;
const uint8_t ERROR_STRING_TOO_SHORT = 1 << 2;
const uint8_t ERROR_IDENTIFIER_NOT_FOUND = 1 << 3;
const uint8_t ERROR_CRC_NOT_FOUND = 1 << 4;
const uint8_t ERROR_PAYLOAD_NOT_FOUND = 1 << 5;

const uint8_t BUFFERSIZE = 128;
const uint8_t IDENTIFIER_STRING_SIZE = 4; // TXT + \0
const uint8_t CRC_STRING_SIZE = 3; // 3e + \0

class BSD{
public:
  //Constructor using standard Serial input
  BSD(HardwareSerial& serial=Serial, const unsigned long baudrate=9600)
    : serial_{serial}, baudrate_{baudrate} {
  }

  void begin(const unsigned long baudrate=0);
  void process();
  bool parse_buffer(const char* buffer, const uint8_t size);
  uint8_t get_identifier(char* identifierString, const char* buffer, const uint8_t size);
  uint8_t get_crc(char* crcString, const char* buffer, const uint8_t size);
  uint8_t compute_crc(uint8_t *crc, const char* buffer, const uint8_t size);
  void print();
  
private:
  HardwareSerial& serial_;
  unsigned long baudrate_;
};

#endif
