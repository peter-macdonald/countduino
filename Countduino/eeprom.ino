#include <EEPROM.h>
#include <ARDUINO.h>
#define MAGIC 0x42
#define MAX_ADDR  1023

word cur_addr;

int init_mem(){
  cur_addr = 0;
  while ( EEPROM.read(cur_addr) != 0xFF )
  	cur_addr ++;
}

void set_cur_addr(word addr){
  cur_addr = addr;
  return;
}

word get_cur_addr(){
  return cur_addr;
}


word eread_word(int addr){
  if ( addr > MAX_ADDR || addr < 0 ) return -1;
  word r = 0;
  byte in;
  in = EEPROM.read(addr);
  r = in;
  in = EEPROM.read(addr + 1);
  r += in << 8;
  return r;
}

void ewrite_word(word data, int addr){
  if ( addr > MAX_ADDR || addr < 0 ) return;
  Serial.print("\n[W] ADDR:");
  Serial.print(addr,HEX);
  Serial.print("\tDATA:");
  Serial.print((byte)(data & 0xff),HEX);
  Serial.print(", ");
  Serial.print((byte)(data >> 8),HEX);
  
  EEPROM.write(addr, (byte)(data & 0xff));
  EEPROM.write(addr + 1, (byte)(data >> 8));
}

int ewrite1(byte data){
  word addr = get_cur_addr();
  if ( addr > MAX_ADDR || addr < 0 ) return -1;
  Serial.print("\n[W] ADDR:");
  Serial.print(addr,HEX);
  Serial.print("\tDATA:");
  Serial.print(data,HEX);
  
  EEPROM.write(addr,data);
  addr ++;
  set_cur_addr(addr);
  return 0;
}

void edump(){
  int i = 0;
  int ending = get_cur_addr();
  Serial.print("\n\n****DUMPING:****\n");
  for( i=0; i < ending; i++ ){
    Serial.print(i,HEX);
    Serial.print("\t");
    Serial.print(EEPROM.read(i),HEX);
    Serial.print("\n");
  }
  Serial.print("****COMPLETE***\n\n");
  return;
}
