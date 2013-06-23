#include <EEPROM.h>

#define MAGIC 0x42

int init_mem(byte flags){
	if ( EEPROM.read(0) != MAGIC ){
		set_cur_addr(2);
	}
	else {
		set_cur_addr(get_cur_addr());
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

void ewrite_word(word data, inta addr){
	EEPROM.write( (byte)(data & 0x0f), addr );
	EEPROM.write( (byte)(data >> 8), addr + 1 );
}

/* Store mon, dom, hr, min */


