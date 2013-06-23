#include <EEPROM.h>
#include <ARDUINO.h>
#define MAGIC 0x42

int init_mem(byte flags){
  if ( EEPROM.read(0) != MAGIC ){
    set_cur_addr(3);
    EEPROM.write(0,0x42);
  }
}

void set_cur_addr(word addr){
  ewrite_word(addr,1);
  return;
}

word get_cur_addr(){
  return eread_word(1);
}

word eread_word(int addr){
  word r = 0;
  byte in;
  in = EEPROM.read(addr);
  r = in;
  in = EEPROM.read(addr + 1);
  r += in << 8;
  return r;
}

void ewrite_word(word data, int addr){
  Serial.print("\n[W] ADDR:");
  Serial.print(addr,HEX);
  Serial.print("\tDATA:");
  Serial.print((byte)(data & 0xff),HEX);
  Serial.print(", ");
  Serial.print((byte)(data >> 8),HEX);
  
  EEPROM.write(addr, (byte)(data & 0xff));
  EEPROM.write(addr + 1, (byte)(data >> 8));
}

void ewrite1(byte data){
  word addr = get_cur_addr();
  Serial.print("\n[W] ADDR:");
  Serial.print(addr,HEX);
  Serial.print("\tDATA:");
  Serial.print(data,HEX);
  
  EEPROM.write(addr,data);
  addr ++;
  set_cur_addr(addr);
  return;
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
