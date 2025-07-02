import abc
import logging
import re
import serial

logger = logging.getLogger(__name__)

ACTIVE = 1
STOPPED = 2
SD = 3

    
class MessageParser(object):
    
    def __init__(self, variable_list=[]):
        self.regex = dict(body=re.compile(r'^\$([^*]+)'),
                          identifier=re.compile(r'^\$([^,]+)'),
                          crc=re.compile(r'\*(.+)$'),
                          payload=re.compile(r',([^*]+)'))
        self.variable_list = variable_list


    def crc(self, s: str) -> str:
        x = 0
        for i in s.encode():
            x ^= i
        crc_string = f"{x:02x}"
        return crc_string
    

    def match(self, key, s):
        m = self.regex[key].search(s)
        if m:
            return m.group(1)
        else:
            return ""
        
    def parse(self, s):
        s_body = self.match("body", s)
        crc = self.match("crc", s).lower() == self.crc(s_body)
        
        s_identifier = self.match("identifier", s)
        s_payload = self.match("payload", s)
        match(s_identifier):
            case 'HI':
                result, result_dict = self.HI(s_payload)
            case 'SD':
                result, result_dict = self.SD(s_payload)
            case 'BY':
                result, result_dict = self.BY(s_payload)
            case 'B':
                result, result_dict = self.BY(s_payload)
            case _:
                result=0, {}
        result_dict['crc'] = crc
        return result, result_dict

    def HI(self, payload):
        return ACTIVE, {}

    def BY(self, payload):
        return STOPPED, {}

    def SD(self, payload):
        result_dict = {}
        fields = payload.split(',')
        for field in fields:
            i, v = field.split(":")
            result_dict[self.variable_list[int(i)]] = v
        return SD, result_dict
            
    
class Extctl(object):


    def __init__(self, device: str, message_parser: MessageParser, baudrate: int = 9600, timeout: int=1):
        logger.info("Initialised Extctl.")
        self.serial: serial.Serial = serial.Serial(device, baudrate, timeout=timeout)
        logger.debug("Serial connection setup.")
        self.message_parser = message_parser
        
        
        
    def read(self):
        line = self.serial.readline()
        if line:
            s = line.decode().strip()
            result, result_dict = self.message_parser.parse(s)
            if result==SD:
                print(result_dict)

if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)


    mp = MessageParser(variable_list=["m_depth", "m_pitch"])
    extctl = Extctl(device="/dev/ttyUSB1",
                    message_parser=mp,
                    baudrate=9600)
    while True:
        extctl.read()



# import re

nmea_string = "$SW,0:0.12,1:508.0*3B"

# match = re.compile(r'\$([^,]+)')
match = re.compile(r'\$([^*]+)')
s=match.search(nmea_string).group(1)

csum=0
for i in s.encode('ascii'):
    csum ^= i
print(hex(csum), nmea_string)
      
