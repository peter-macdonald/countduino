//REPLACE ALL millis() WITH RTC READ EVENTS
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <EEPROM.h>
#include <DS1302.h>

#define LED_PIN 13
#define PIR_PIN 2
#define PIR_INT 0
//#define BUFFER_TIME 15000

// GLOBAL VARS
unsigned long sleep_event_time = 0; // capture time when wake up from sleep
unsigned long elapsed_time = 0;  // time that have been awake for
DS1302 rtc(9, 8, 7);  // initialize the DS1302 RTC on pins 2,3,4 (CE, IO, SCLK)
byte last_write_TS[2] = {0,0};

// ISR to run when interrupted in Sleep Mode
void pin2Interrupt() {/*no operation needed*/}

// Sleep until PIR wakes us up
void sleepUntilInterrupt() {
  delay(50);
  digitalWrite(LED_PIN, LOW);           // indicate we are sleeping
  //sleep_event_time = millis();          // record the time we went to sleep NEEDS RTC CALL
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // setting up for sleep ...
  sleep_enable();                       // setting up for sleep ...
  sleep_mode();                         // now goes to Sleep and waits for the interrupt
  // -- sleeping until next line --
  sleep_disable();                      // disable sleep while awake
  digitalWrite(LED_PIN, HIGH);          // indicate we are awake
}

void setup_RTC() {
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  
  // COMMENT OUT BELOW ONCE CONFIGURED
  /*
  
  rtc.writeProtect(false);

  rtc.setDOW(SATURDAY);        // Set Day-of-Week to SATURDAY
  rtc.setTime(22, 42, 00);     // Set the time to 12:01:15 (24hr format)
  rtc.setDate(12, 21, 2013);   // Set the date to August 6th, 2010

  //@TODO: Should store the time truncation offset, and retrieve it.
  rtc.writeProtect(true);
  
  */
  return;
}

void write_compact_TS() {
  // write a compacted timestamp to the EEPROM storage
  Time t;
  t = rtc.getTime();
  byte c_ts[2] = {0};

  // layout:     [1]-->lsb [0]-->lsb
  //             MMMM DDDD HHHH MMMM
  //  where:
  //    D0-D2      minutes / 10   (0-6)                4 bits
  //    D3-D7      the number of the hour (1-24)       4 bits
  //    D8-D11     the number of the day (1-7)         4 bits
  //    D12-D16    the number of the month (1-12)      4 bits
  //                                                  16 bits = 2 bytes 
  
  //Serial.print("Writing to eeprom theoretically: (min, dour, dow, mon)  ");

  
  c_ts[0] = ((t.min / 10)&0x07) | ((t.hour)<<3);
  c_ts[1] = ((t.dow)&0x0f) | ((t.mon)<<4);
  
  if ( c_ts[0] != last_write_TS[0] || c_ts[1] != last_write_TS[1] ){
    last_write_TS[0] = c_ts[0];
    last_write_TS[1] = c_ts[1];
    
    ewrite1(c_ts[0]);
    ewrite1(c_ts[1]);
    
    Serial.print(t.hour);
    Serial.print(":");
    Serial.print(t.min);
    Serial.print(", DOW ");
    Serial.print(t.dow);
    Serial.print(", Mon ");
    Serial.print(t.mon);
    
  } else {
    Serial.println(" ...Not written");
  }
  
  return;
}

// Outputs the compact TS at start_addr as a json object. Assumes 1 level of indentation
void read_compact_TS(word start_addr) {
  byte c_ts[2] = {0};
  int minu, hr, day, month;

  // layout:     [1]-->lsb [0]-->lsb
  //             MMMM DDDD HHHH MMMM
  //  where:
  //    D0-D2      minutes / 10   (0-6)                4 bits
  //    D3-D7      the number of the hour (1-24)       4 bits
  //    D8-D11     the number of the day (1-7)         4 bits
  //    D12-D16    the number of the month (1-12)      4 bits
  //                                                  16 bits = 2 bytes 

  c_ts[0] = eread1(start_addr);
  c_ts[1] = eread1(start_addr + 1);

  minu = (c_ts[0] & 0x07) * 10;
  hr = ((c_ts[0]>>3) & 0x1F);
  day = (c_ts[1] & 0x0F);
  month = ((c_ts[1]>>4) & 0x0F);
  
  Serial.println("{");   // Start object
  
  Serial.print("\"Month\":");
  Serial.print(month);
  Serial.println(",");
  // Jan is 1
  
  Serial.print("\"DOW\":");
  Serial.print(day);
  Serial.println(",");
  // Monday is 1
  
  Serial.print("\"Hour\":");
  Serial.print(hr);
  Serial.println(",");
  
  Serial.print("\"Minute\":");
  Serial.print(minu);
  Serial.print("");
  
  Serial.println("}");   // End object
  return;
}

void output_as_JSON(){
  word last_addr;
  int i;
  last_addr = get_cur_addr();
  
  Serial.println("\n\n{");
  for (i = 0; i < last_addr; i+=2 ){
    // Delimit the fields, but not the first or last
    if ( i > 0 )
      Serial.println(",");
    
    // Output the TS name field ("TS-#" : )
    Serial.print("\"TS-");
    Serial.print(i/2);
    Serial.print("\" : ");
    
    // Output the JSON of a given TS
    read_compact_TS(i);
  }
  Serial.println("}");
  
  return;
}

void setup() {
  // Initializations
  Serial.begin(115200); 
  setup_RTC();
  init_mem();
  
  // Set up pins
  pinMode(LED_PIN, OUTPUT);
  pinMode (PIR_PIN, INPUT);
  attachInterrupt(PIR_INT, pin2Interrupt, HIGH);
 
  Serial.println(" -------------  STARTJSON  -------------- ");
  
  // Output the current time stamps as JSON
  output_as_JSON();
  
  Serial.println(" -------------  ENDJSON  -------------- ");
  
  delay(50);
  
  // Clear memeory (uncomment to clear, make sure to recomment!) ONLY DO THIS IF WANT TO, USER CONTROL
   /*
  
  word last_addr;
  int i;
  last_addr = get_cur_addr();
  set_cur_addr(0);
  //last_addr = 0x3bb+1;
  for (i = 0; i < last_addr; i++){
    ewrite1(0xFF);
  }
  set_cur_addr(0);
  
  /**/
  
  
  // Make it so that it doesn't trigger in the first 4 minutes
  Time t;
  t = rtc.getTime();
  last_write_TS[0] = ((t.min / 10)&0x07) | ((t.hour)<<3);
  last_write_TS[1] = ((t.dow)&0x0f) | ((t.mon)<<4);
  
}

void loop() {
  // start while asleep
  Serial.print("SLEEPING");
  sleepUntilInterrupt();
  
  while(1) {
    Serial.print("\nTRIGGER EVENT at ");
    write_compact_TS();
      
    // go back to sleep
    sleepUntilInterrupt();
  }
}
