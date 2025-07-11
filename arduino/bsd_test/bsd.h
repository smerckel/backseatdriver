#ifndef __BSD_H__
#define __BSD_H__

#include <Arduino.h>
#include "Base64.h"
#include <SoftwareSerial.h>

const uint8_t NO_ERROR = 0;
const uint8_t ERROR_NO_DOLLAR_SIGN = 1 << 0;
const uint8_t ERROR_STRING_TOO_SHORT = 1 << 1;
const uint8_t ERROR_IDENTIFIER_NOT_FOUND = 1 << 2;
const uint8_t ERROR_CRC_NOT_FOUND = 1 << 3;
const uint8_t ERROR_PAYLOAD_NOT_FOUND = 1 << 4;

// Status bits
const uint8_t INACTIVE = 0;
const uint8_t ACTIVE = 1 << 0;
const uint8_t CONFIGFILEREQUESTED = 1 << 1;
const uint8_t DATABLOCKRECEIVED = 1 << 2;
const uint8_t FILETRANSFERCOMPLETE = 1 << 3;
const uint8_t DO_SD = 1 << 4;

const uint8_t INPUTBUFFERSIZE = 64;
const uint8_t IDENTIFIER_STRING_SIZE = 4; // TXT + \0
const uint8_t CRC_STRING_SIZE = 3; // 3e + \0
const uint8_t DECODEDSTRINGSIZE=34; // extctl.ini defines 32.


const uint8_t MAXKEYWORDSIZE = 12; // fits threshold/nprofiles
const uint8_t MAXVALUESIZE = 12; 
const uint8_t EXPECTINGKEYWORD = 1 << 0;
const uint8_t EXPECTINGVALUE = 1 << 1;
const uint8_t PROCESSKEYVALUEPAIR = 1 << 2;
const uint8_t ISCOMMENT = 1 << 3;
 

class BSD{
public:
  //Constructor using standard Serial input

  BSD(const char* configFilename,
      SoftwareSerial& monitor,
      HardwareSerial& serial=Serial,
      const unsigned long baudrate=9600)
    : configFilename_{configFilename}, monitor_{monitor}, serial_{serial},
      baudrate_{baudrate}, p_{0}, status_{INACTIVE},
      sci_water_pressure_{-99}, sci_water_temp_{-99}{
  }

  void begin(const unsigned long baudrate=0);
  
  void process();

  void processInputBuffer();
  
  uint8_t parseBuffer(const char *buffer);
  
  uint8_t getIdentifier(char* identifierString,
			uint8_t *pPayload,
			const char* buffer,
			const uint8_t size);
  
  uint8_t getCrc(char* crcString,
		 uint8_t *pPayload,
		 const char* buffer, 
		 const uint8_t size);
  
  uint8_t computeCrc(uint8_t *crc,
		      const char* buffer,
		      const uint8_t size);

  void sendSWmessage(uint8_t index, int value);
  void sendSWmessage(uint8_t index, float value);

  void parsePayloadSD(const char* buffer, uint8_t p0, uint8_t p1);

  void updateMissionParameters();
  
  void requestConfigFile();

  void sendGoCommand();

  void readFileData(const char* encodedString);

  void decodeBase64(char* decodedString, const char* encodedString);
  
private:
  HardwareSerial& serial_;
  SoftwareSerial& monitor_;
  const unsigned long baudrate_;
  const char* configFilename_;

  // The input buffer and its pointer.
  char inputBuffer_[INPUTBUFFERSIZE];
  uint8_t p_;

  uint8_t status_;

  // Variables that we read from the glider. The order has to
  // correspond with the order of variables in the extctl.ini
  // file. These variables should go into their own class someday.
  
  float sci_water_pressure_;
  float sci_water_temp_;
  
  // Variables that control the flight logic. They should go into their own class someday.
  float dmin_;
  float dmax_;
  uint8_t threshold_;
  float Twarm_;
  float Tcold_;
};


#endif
