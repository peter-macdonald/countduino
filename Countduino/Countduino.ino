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

  rtc.setDOW(TUESDAY);        // Set Day-of-Week to SATURDAY
  rtc.setTime(9, 46, 00);     // Set the time to 12:01:15 (24hr format)
  rtc.setDate(15, 7, 2013);   // Set the date to August 6th, 2010

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
  //    D0-D3      minutes / 4   (0-15)                4 bits
  //    D4-D7      the number of the hour (1-12)       4 bits
  //    D8-D11     the number of the day (1-7)         4 bits
  //    D12-D16    the number of the month (1-12)      4 bits
  //                                                  16 bits = 2 bytes 
  
  //Serial.print("Writing to eeprom theoretically: (min, dour, dow, mon)  ");
  Serial.print(t.hour);
  Serial.print(":");
  Serial.print(t.min);
  Serial.print(", DOW ");
  Serial.print(t.dow);
  Serial.print(", Mon ");
  Serial.print(t.mon);
  
  c_ts[0] = ((t.min / 4)&0x0f) | ((t.hour)<<4);
  c_ts[1] = ((t.dow)&0x0f) | ((t.mon)<<4);
  
  if ( c_ts[0] != last_write_TS[0] || c_ts[1] != last_write_TS[1] ){
    last_write_TS[0] = c_ts[0];
    last_write_TS[1] = c_ts[1];
    
    ewrite1(c_ts[0]);
    ewrite1(c_ts[1]);
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
  //    D0-D3      minutes / 4   (0-15)                4 bits
  //    D4-D7      the number of the hour (1-12)       4 bits
  //    D8-D11     the number of the day (1-7)         4 bits
  //    D12-D16    the number of the month (1-12)      4 bits
  //                                                  16 bits = 2 bytes 

  c_ts[0] = eread1(start_addr);
  c_ts[1] = eread1(start_addr + 1);

  minu = (c_ts[0] & 0x0F) * 4;
  hr = ((c_ts[0]>>4) & 0x0F);
  day = (c_ts[1] & 0x0F);
  month = ((c_ts[1]>>4) & 0x0F);
  
  Serial.println("{");   // Start object
  
  Serial.print("\t\t\"Month\" : ");
  switch (month){
    case  1: Serial.println("\"Jan\" ,"); break;
    case  2: Serial.println("\"Feb\" ,"); break;
    case  3: Serial.println("\"Mar\" ,"); break;
    case  4: Serial.println("\"Apr\" ,"); break;
    case  5: Serial.println("\"May\" ,"); break;
    case  6: Serial.println("\"Jun\" ,"); break;
    case  7: Serial.println("\"Jul\" ,"); break;
    case  8: Serial.println("\"Aug\" ,"); break;
    case  9: Serial.println("\"Sep\" ,"); break;
    case 10: Serial.println("\"Oct\" ,"); break;
    case 11: Serial.println("\"Nov\" ,"); break;
    case 12: Serial.println("\"Dec\" ,"); break;
    default: Serial.println("\"Unknown\" ,");
  }
  
  Serial.print("\t\t\"DOW\" : ");
  switch (day){
    case 1: Serial.println("\"Mon\" ,"); break;
    case 2: Serial.println("\"Tue\" ,"); break;
    case 3: Serial.println("\"Wed\" ,"); break;
    case 4: Serial.println("\"Thu\" ,"); break;
    case 5: Serial.println("\"Fri\" ,"); break;
    case 6: Serial.println("\"Sat\" ,"); break;
    case 7: Serial.println("\"Sun\" ,"); break;
    default: Serial.println("\"Unknown\" ,");
  }
  
  Serial.print("\t\t\"Hour\" : ");
  Serial.print(hr);
  Serial.println(" , ");
  
  Serial.print("\t\t\"Minute\" : ");
  Serial.print(minu);
  Serial.println("");
  
  Serial.println("\t}");   // End object
  
  /*
  Serial.print("TimeStamp: ");
  Serial.print(mon);
  Serial.print(" month, ");
  Serial.print(day);
  Serial.print(" day of week, at about ");
  Serial.print(hour);
  Serial.print(":"); 
  Serial.print(minu);
  Serial.print(" (+-4 min)\n");
  */

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
  Serial.begin(9600); 
  setup_RTC();
  init_mem();
  
  // Set up pins
  pinMode(LED_PIN, OUTPUT);
  pinMode (PIR_PIN, INPUT);
  attachInterrupt(PIR_INT, pin2Interrupt, HIGH);
 
  //Output raw memeory of used section on startup
  //edump();
  
  // Output the current time stamps as JSON
  output_as_JSON();
  
  Serial.print("\n\n -------------  END OF JSON  -------------- \n\n");
  
  delay(50);
  
  // Clear memeory (uncomment to clear, make sure to recomment!)
  /*
  word last_addr;
  int i;
  last_addr = get_cur_addr();
  set_cur_addr(0);
  for (i = 0; i < last_addr; i++){
    ewrite1(0xFF);
  }*/
  
  // Make it so that it doesn't trigger in the first 4 minutes
  Time t;
  t = rtc.getTime();
  last_write_TS[0] = ((t.min / 4)&0x0f) | ((t.hour)<<4);
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
    delay(1000);
    sleepUntilInterrupt();
  }
}
