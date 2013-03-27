/*
 * IOT-Clock project

  Copyright (c) 2013 Vino Rodrigues, Tecsmith

  This work is licensed under the Creative Commons ShareAlike 1.0 Generic (CC SA 1.0) License
  To view a copy of this license, visit http://creativecommons.org/licenses/sa/1.0 or send a
  letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 */

#define ETHERSHEILD

// Arduino libs
#include <SPI.h>           // http://arduino.cc/en/Reference/SPI
#include <Dns.h>           // part of Ethernet library
#include <Wire.h>          // http://arduino.cc/en/Reference/Wire
#include <avr/wdt.h>       // for reboot() function
#include <avr/pgmspace.h>  // for PROGMEM
// 3rd-party libs
#include <Time.h>          // http://playground.arduino.cc/Code/time
#include <Timezone.h>      // http://github.com/JChristensen/Timezone
#include <DMD.h>           // http://github.com/freetronics/DMD
#include <TimerOne.h>      // http://code.google.com/p/arduino-timerone
#ifdef ETHERSHEILD
#include <Ethernet.h>      // http://arduino.cc/en/Reference/Ethernet
#endif
// Project libs
#include "Buttons.h"       // ButtonDebouncer
#include "RTC.h"
#ifdef ETHERSHEILD
#include "NTP.h"           // NTP
#endif
#include "font8.h"
#include "font9.h"
#include "AppChange.h"     // change time / date app
#include "AppDigital.h"    // digital app
#include "AppAnalog.h"     // analog app
#include "AppDate.h"       // date app
#include "font12.h"
#include "AppLarge.h"      // large app

// Debouncer
static ButtonDebouncer debouncer;

#ifdef ETHERSHEILD
// init Ethernet libs
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip( 192, 168, 1, 13 );
IPAddress ip_dns( 192, 168, 1, 1 );
const char serverName1[] PROGMEM = "0.pool.ntp.org";
const char serverName2[] PROGMEM = "1.pool.ntp.org";
const char serverName3[] PROGMEM = "2.pool.ntp.org";

DNSClient DNS = DNSClient();
#endif

// ***** Globals *****
int mode = 0;
unsigned long last_m;

bool featureRtc = false;
#ifdef ETHERSHEILD
bool featureEth = false;
bool featureDns = false;
bool int_0 = false;
#endif

byte piezo = 5;  // pin for speaker
byte anlgbtn1 = A2;
byte anlgbtn2 = A3;

/**
 * Arduino Setup
 */
void setup() {
  mode = 0;
  
  // init DMD
  // init TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize(5000);  // period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.attachInterrupt(ScanDMD);  // attach Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

  dmd.clearScreen(false);

  // Detect RTC
  featureRtc = RTC.available();
  if (featureRtc) {
    RTC.begin();
    setSyncInterval(48);  /// get time from RTC every 48 sec
    setSyncProvider(RTC.get);
  }

  debouncer = ButtonDebouncer(btnReader, 100);

  #ifdef ETHERSHEILD
  // start Ethernet and UDP
  Ethernet.begin(mac, ip, ip_dns);  // using DHCP makes the sketch too big!
  delay(1000);  // give the Ethernet shield a second to initialize
  featureEth = (Ethernet.localIP()[3] != 0);
  if (featureEth) {
    DNS.begin(ip_dns);
  }
  featureDns = false;
  #endif

  #ifdef ETHERSHEILD
  // setup alarm
  int_0 = true;
  attachInterrupt(0, alarmTrigger, CHANGE);
  tmElements_t tm;  tm.Hour = 4;  tm.Minute = 15;
  RTC.setAlarm2(tm);
  #endif

  dmd.clearScreen(true);

  // init Speaker
  pinMode(piezo, OUTPUT);
  beep();
}

/**
 * Arduino Loop
 */
void loop() {
  uint16_t bc;
  unsigned long m;

  #ifdef ETHERSHEILD
  if (int_0) {
    int_0 = false;
    RTC.resetAlarm2();
    syncTime();
  }
  #endif

  // Button code
  bc = debouncer.read();
  if (bc != 0) {
    switch (bc) {
      case 1: setMode(mode + 1); break;
      case 2: setMode(mode - 1); break;
      case 11:
      case 12:
      case 21:
      case 22: 
        ChangeApp.key(bc, mode);
        break;
      case 30:
        setMode( ChangeApp.cancel() );
        break;
      case 33: reboot(); break;
    }
  }
  
  // Display code
  m = millis();
  if (m < last_m) last_m = 0;  // millis() has overflowed, will occure aprox once in 50 days
  if (m > (last_m + 100)) {  // only scan/display time every 100ms
    App::getTime();
    switch (mode) {
      case MODE_DIGITAL: DigitalApp.loop(); break;
      case MODE_ANALOG: AnalogApp.loop(); break;
      case MODE_DATE: DateApp.loop(); break;
      case MODE_LARGE: LargeApp.loop(); break;
      case MODE_SET_TIME:
      case MODE_SET_DATE:
        ChangeApp.loop();
        break;
      default: setMode(MODE_START); break;
      }
  }

  delay(50);
}

/** 
 * Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
 * called at the period set in Timer1.initialize();
 */
void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

/**
 * Beep
 */
void beep() {
  tone(piezo, 1000, 50);
}

void reboot() {
  cli();  // Clear interrupts
  wdt_enable(WDTO_15MS);  // Set the Watchdog to 15ms
  while (true) ;  // Enter an infinite loop
}

/**
 * Read and convert anaolg button press, and provide as callback to ButtonDebouncer
 */
void btnReader(uint16_t &value) {
  uint8_t b2 = (analogRead(anlgbtn2) / 100) + 1;
  switch (b2) {
     case 6: value = 1; break;
     case 5: value = 2; break;
     case 1: value = 3; break;
     default: value = 0; break;
  }

  uint8_t b1 = (analogRead(anlgbtn1) / 100) + 1;
  switch (b1) {
    case 6: value += 10; break;
    case 5: value += 20; break;
    case 1: value += 30; break;
  }
}

void setMode(int newMode) {
  if ((mode == MODE_SET_TIME) || (mode == MODE_SET_DATE)) {
    beep();
    newMode = ChangeApp.save();
  }

  if (newMode > MODE_MAX) newMode = 1;
  if (newMode < 1) newMode = MODE_MAX;

  mode = newMode;

  switch (mode) {
    case MODE_DIGITAL: DigitalApp.reset(); break;
    case MODE_ANALOG: AnalogApp.reset(); break;
    case MODE_DATE: DateApp.reset(); break;
    case MODE_LARGE: LargeApp.reset(); break;
  }
}

#ifdef ETHERSHEILD
void alarmTrigger()  // Triggered when alarm interupt fired
{
  int_0 = true;
}
#endif

/**
 * Sync RTC time to NTP time if required
 */
void syncTime() {
  #ifdef ETHERSHEILD
  if ((!featureEth) || (!featureRtc))  return;

  if (!featureDns) {
    IPAddress ts1, ts2, ts3;
    featureDns = true;
    DNS.getHostByName(strcpy_P(buff, serverName1), ts1);
    DNS.getHostByName(strcpy_P(buff, serverName2), ts2);
    DNS.getHostByName(strcpy_P(buff, serverName3), ts3);
    NTP.begin(ts1, ts2, ts3);
  }

  time_t t_ntp, t_rtc;
  t_ntp = NTP.get();
  if (t_ntp != 0) {
    t_rtc = RTC.get();
    if (t_ntp != t_rtc) RTC.set(t_ntp);  // set the RTC time to NTP time
  }
  #endif
}

/* eof */
