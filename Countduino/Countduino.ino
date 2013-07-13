//REPLACE ALL millis() WITH RTC READ EVENTS
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <EEPROM.h>
#include <DS1302.h>

#define LED_PIN 13
#define PIR_PIN 2
#define PIR_INT 0
#define BUFFER_TIME 15000

// GLOBAL VARS
unsigned long sleep_event_time = 0; // capture time when wake up from sleep
unsigned long elapsed_time = 0;  // time that have been awake for
DS1302 rtc(2, 3, 4);  // initialize the DS1302 RTC on pins 2,3,4 (CE, IO, SCLK)

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
  rtc.writeProtect(false);

   // COMMENT OUT BELOW ONCE CONFIGURED
  rtc.setDOW(FRIDAY);        // Set Day-of-Week to FRIDAY
  rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
  rtc.setDate(6, 8, 2010);   // Set the date to August 6th, 2010

  //@TODO: Should store the time truncation offset, and retrieve it.
  rtc.writeProtect(true);
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

  c_ts[0] = ((t.min / 4)&0x0f) | ((t.hour)<<4);
  c_ts[1] = ((t.dow)&0x0f) | ((t.mon)<<4);

  ewrite1(c_ts[0]);
  ewrite1(c_ts[1]);
  
  return;
}

void setup() {
  Serial.begin(9600); 
  setup_RTC();

  pinMode(LED_PIN, OUTPUT);
  pinMode (PIR_PIN, INPUT);
  attachInterrupt(PIR_INT, pin2Interrupt, HIGH);
 
  init_mem();
  //dump current logs to serial on startup
  edump();
  delay(50);
}

void loop() {
  // start while asleep
  Serial.print("SLEEPING");
  sleepUntilInterrupt();
  
  while(1) {
    //if((millis() - sleep_event_time) >= BUFFER_TIME) {
      Serial.print("\nVALID TRIGGER EVENT, write to EEPROM:");
      write_compact_TS();
    //}
    // go back to sleep
    delay(1000);
    sleepUntilInterrupt();
  }
}
