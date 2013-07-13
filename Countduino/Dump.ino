//REPLACE ALL millis() WITH RTC READ EVENTS
#include <EEPROM.h>

void read_compact_TS(word start_addr) {
  byte c_ts[2] = {0};
  int min, hour, day, mon;

  // layout:     [1]-->lsb [0]-->lsb
  //             MMMM DDDD HHHH MMMM
  //  where:
  //    D0-D3      minutes / 4   (0-15)                4 bits
  //    D4-D7      the number of the hour (1-12)       4 bits
  //    D8-D11     the number of the day (1-7)         4 bits
  //    D12-D16    the number of the month (1-12)      4 bits
  //                                                  16 bits = 2 bytes 

  c_ts[0] = eread1(start_addr);
  c_ts[1] = eread1(start_addr + 1);

  min = (c_ts[0] & 0x0F) * 4;
  hour = ((c_ts[0]>>4) & 0x0F);
  day = (c_ts[1] & 0x0F);
  mon = ((c_ts[1]>>4) & 0x0F);
  

  Serial.print("TimeStamp: %2d month, %2d day of week, at about %2d:%2d (+-4 min)\n", mon, day, hour, min);

  return;
}

void setup() {
  Serial.begin(9600); 

  init_mem();
  Serial.print("\nREADING TIME STAMPS FROM EEPROM:\n");
  
  word last_addr;
  int i;
  for (i = 0; i < last_addr; i+=2 ){
  	read_compact_TS(i);
  }
}

void loop() {
	return;
}
