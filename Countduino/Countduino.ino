//REPLACE ALL millis() WITH RTC READ EVENTS
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <EEPROM.h>

#define LED_PIN 13
#define PIR_PIN 2
#define PIR_INT 0
#define BUFFER_TIME 15000

// MAIN PROGRAM
unsigned long sleep_event_time = 0; // capture time when wake up from sleep
unsigned long elapsed_time = 0;  // time that have been awake for

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

void setup() {
  Serial.begin(9600); 
  Serial.print("SLEEPING");
  pinMode(LED_PIN, OUTPUT);
  pinMode (PIR_PIN, INPUT);
  attachInterrupt(PIR_INT, pin2Interrupt, HIGH);
 
  init_mem(0);
  //dump current logs to serial on startup
  edump();
  delay(50);
}

void loop() {
  // start while asleep
  sleepUntilInterrupt();
  
  while(1) {
    //if((millis() - sleep_event_time) >= BUFFER_TIME) {
      Serial.print("\nVALID TRIGGER EVENT, write to EEPROM:");
      ewrite1(0xA1);
    //}
    // go back to sleep
    delay(1000);
    sleepUntilInterrupt();
  }
}
