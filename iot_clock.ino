/*
 * IOT-Clock project

  Copyright (c) 2013 Vino Rodrigues, Tecsmith

  This work is licensed under the Creative Commons ShareAlike 1.0 Generic (CC SA 1.0) License
  To view a copy of this license, visit http://creativecommons.org/licenses/sa/1.0 or send a
i  letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 */

// Arduino libs
#include <SPI.h>           // http://arduino.cc/en/Reference/SPI
#include <Ethernet.h>      // http://arduino.cc/en/Reference/Ethernet
#include <Dns.h>           // part of Ethernet library
#include <Wire.h>          // http://arduino.cc/en/Reference/Wire
#include <avr/wdt.h>       // for reboot() function
// 3rd-party libs
#include <Time.h>          // http://playground.arduino.cc/Code/time
#include <Timezone.h>      // http://github.com/JChristensen/Timezone
#include <DMD.h>           // http://github.com/freetronics/DMD
#include <TimerOne.h>      // http://code.google.com/p/arduino-timerone
#include <DS3232RTC.h>     // http://github.com/Tecsmith/DS3232RTC
// Project libs
#include "NTP.h"           // NTP
#include "Buttons.h"       // ButtonDebouncer
#include "Font6x10.h"
#include "Font4x5.h"
#include "Font12pt.h"

// init DMD libs
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// Debouncer
static ButtonDebouncer debouncer;

// AU Eastern Time Zone (Sydney, NSW)
TimeChangeRule zoneSTD = {"AEST", First, Sun, Apr, 2, +600};  // Standard time
TimeChangeRule zoneDST = {"AEDT", First, Sun, Oct, 2, +660};  // Daylight time
static Timezone timezone(zoneDST, zoneSTD);

// init Ethernet libs
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip( 192, 168, 1, 55 );
IPAddress ip_dns( 192, 168, 1, 1 );

static char serverName1[] = "0.pool.ntp.org";
static char serverName2[] = "1.pool.ntp.org";
static char serverName3[] = "2.pool.ntp.org";

DNSClient DNS = DNSClient();


// ***** Globals *****
char buff[127];
int mode = 0;
int oldMode = 1;
tmElements_t time;
bool isAvo, isDst;
tpElements_t temp;
int HH, MM, Sec, DD, NN, YY, TT, Td = 99;
unsigned long last_m;

bool featureRtc = false;
bool featureEth = false;

byte piezo = 5;  // pin for speaker
byte anlgbtn1 = A2;
byte anlgbtn2 = A3;

static char MSG_HELLO[] = "Hello";
static char MSG_MODE[] = "Mode %d";

#define round(x) ((x >= 0) ? floor(x + 0.5) : ceil(x - 0.5))

/**
 * Arduino Setup
 */
void setup() {
  /*
  Serial.begin(9600);  // TODO : Remove post debug
  while (!Serial) { ; }  // wait for serial port to connect. Needed for Leonardo only    // TODO : Remove post debug
  */

  // init Speaker
  pinMode(piezo, OUTPUT);
  beep();

  // init DMD
  // init TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize(5000);  // period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.attachInterrupt(ScanDMD);  // attach Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

  dmd.clearScreen(false);

  // Initalize and/or read memory
  /*
  SRAM.seek(0);
  for (int i = 0; i < 2; i++) buff[i] = SRAM.read();
  if ((buff[0] != 'T') || (buff[1] != 'S')) {
    // uninitialized, so lets do so now...
    SRAM.seek(0);
    SRAM.write(84);
    SRAM.write(83);
  }
  */

  // Detect RTC
  featureRtc = RTC.available();
  if (featureRtc) {
    RTC.set33kHzOutput(false);
    setSyncInterval(60);  /// get time from RTC every minute
    setSyncProvider(RTC.get);
  }

  debouncer = ButtonDebouncer(btnReader, 100);

  // start Ethernet and UDP
  Ethernet.begin(mac, ip, ip_dns);  // using DHCP makes the sketch too big!
  delay(1000);  // give the Ethernet shield a second to initialize
  featureEth = (Ethernet.localIP()[3] != 0);
  if (featureEth) {
    DNS.begin(ip_dns);
  }

  // init Timezone
  timezone = Timezone(zoneDST, zoneSTD);

  mode = 0;
  oldMode = 1;

  reset();
  beep();
}

/**
 * Arduino Loop
 */
void loop() {
  int bc;
  unsigned long m;

  // Button code
  bc = debouncer.read();
  if (bc != 0) {
    switch (bc) {
      case 1: setMode(mode + 1); break;
      case 2: setMode(mode - 1); break;
      case 33: reboot(); break;
    }
  }
  
  // Display code
  m = millis();
  if (m < last_m) last_m = 0;  // millis() has overflowed, will occure aprox once in 50 days
  if (m > (last_m + 250)) {  // only scan/display time every 250ms
    switch (mode) {
    case 1:
      mode1();
      break;
    case 2:
      mode2();
      break;
   default:
       mode0();
      break;
    }
  }

  delay(50);
}

void reboot() {
  cli();  // Clear interrupts
  wdt_enable(WDTO_15MS);  // Set the Watchdog to 15ms
  while (true) ;  // Enter an infinite loop
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
  tone(piezo, 1109, 100);
}

/**
 * Return true if the number is Odd
 */
boolean isOdd(int num) {
  return (num & 1);
}

/*
 * Display marquee buffer, of given length with variable delay 
 */
void doMarquee(char *buff, byte length, int delay) {
  long start, timer;
  boolean ret;

  dmd.selectFont(Font12pt);
  dmd.drawMarquee(buff, length, 32, 2);

  start = millis();  timer = start;  ret = false;
  while(!ret){
    if ((timer + delay) < millis()) {
      ret = dmd.stepMarquee(-1,0);
      timer = millis();
    }
  } 
}

/**
 * Display marquee with variable delay
 */
void doMarquee(String msg, int delay) {
  size_t n;

  n = 0;
  for (byte i = 0; i < msg.length(); i++) {
    buff[i] = msg[i];
  }

  doMarquee(buff, msg.length(), delay);
}

/**
 * Display marquee with 25ms delay
 */
void doMarquee(String msg) {
  doMarquee(msg, 25);
}

/**
 * Read and convert anaolg button press, and provide as callback to ButtonDebouncer
 */
void btnReader(uint16_t &value) {
  uint8_t b1, b2;
  b1 = (analogRead(anlgbtn1) / 100) + 1;
  switch (b1) {
    case 6: value = 10; break;
    case 5: value = 20; break;
    case 1: value = 30; break;
    default: value = 0; break;
  }
  b2 = (analogRead(anlgbtn2) / 100) + 1;
  switch (b2) {
     case 6: value += 1; break;
     case 5: value += 2; break;
     case 1: value += 3; break;
     // default ~ do nothing
  }
}

/**
 * The start-up sequence
 */
void testPattern() {
  // #2
  dmd.clearScreen(true);
  doMarquee(MSG_HELLO, 75);

  // #1
  dmd.clearScreen(true);
  for (int i=4; i>0; i--) {
    dmd.drawTestPattern(i-1);
    delay(500);
  }
  dmd.clearScreen(false);

  delay(500);
}

void setMode(int newMode) {
  setMode(newMode, false);
}

void setMode(int newMode, bool silent) {
  int len;

  if (newMode > 2) newMode = 1;
  if (newMode < 1) newMode = 2;

  oldMode = mode;
  mode = newMode;

  if (!silent) {
    dmd.clearScreen(true);
    len = sprintf(buff, MSG_MODE, mode);
    doMarquee(buff, len, 25);
  }

  reset();
}

/**
 * Reset place holders and clear screen
 */
void reset() {
  // clear/init the DMD pixels held in RAM
  dmd.clearScreen(true);   //true is normal (all pixels off), false is negative (all pixels on)
  HH = 99;  MM = 99;  Sec = 99;  DD = 99;  NN = 99;  YY = 99;  TT = 99;
}

/**
 * Mode 0 is the once only start up mode
 */
void mode0() {
  testPattern();

  syncTime();

  dmd.clearScreen(true);

  setMode(oldMode);
}

/**
 * Read the current time
 */
void getTime() {
  time_t local = timezone.toLocal( now() );
  isDst = timezone.locIsDST(local);
  breakTime(local, time);
  isAvo = (time.Hour >= 12);
  if (isAvo) time.Hour -= 12;
  if (time.Hour == 0) time.Hour = 12;
}

/**
 * Read the current temprature
 */
void getTemp() {
  RTC.readTemperature(temp);
}

/**
 * Sync RTC time to NTP time if required
 */
void syncTime() {
  IPAddress ts1, ts2, ts3;
  time_t t_ntp, t_rtc;
  tmElements_t te;  // debug

  if (featureEth) {
    DNS.getHostByName(serverName1, ts1);
    DNS.getHostByName(serverName2, ts2);
    DNS.getHostByName(serverName3, ts3);
    NTP.begin(ts1, ts2, ts3);
  }

  if (featureEth && featureRtc) {
    t_ntp = NTP.get();
    t_rtc = RTC.get();

    if (t_ntp != t_rtc) RTC.set(t_ntp);  // set the RTC time to NTP time
  } 
}

/**
 * Mode 1 shows digital clock in 7x10 on top and temp (odd mins) / date (even mins) in 4x5 at the bottom
 */
void mode1() {
  int h1, m1, s1, xo;
  boolean m_c;

  getTime();
  h1 = time.Hour;
  m1 = time.Minute;
  s1 = time.Second;

  // will be drawing, so set up DMD font  
  if ((m1 != MM) || (h1 != HH)) dmd.selectFont(Font6x10);

  if (h1 != HH) {
    dmd.clearScreen(true);

    dmd.drawFilledBox(0, 12, 1, 13, (isAvo ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
    dmd.drawFilledBox(30, 12, 32, 13, (isDst ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));

    xo = (h1 > 9) ? 1 : 5;

    // display hour
    HH = h1;
    sprintf(buff, "%d", h1);
    if (h1 > 9) {
      dmd.drawString(xo, 0, buff, 2, GRAPHICS_NORMAL);
    } else {
      dmd.drawString(xo, 0, buff, 1, GRAPHICS_NORMAL);
    } 
  }

  xo = (h1 > 9) ? 15 : 12;

  m_c = (m1 != MM);
  if (m_c) {
    // display minutes
    MM = m1;
    sprintf(buff, "%02d", MM);
    dmd.drawString(xo+3, 0, buff, 2, GRAPHICS_NORMAL);
  }
  
  if (s1 != Sec) {
    // display secs (or the flashing ':')
    Sec = s1;
    dmd.drawFilledBox(xo, 2, xo+1, 3, (isOdd(s1) ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
    dmd.drawFilledBox(xo, 6, xo+1, 7, (isOdd(s1) ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
  }

  // clear bottom data area
  if (m_c) {
    // clear bottom area
    dmd.drawFilledBox(2, 11, 29, 15, GRAPHICS_INVERSE);
    dmd.selectFont(Font4x5);

    if (isOdd(MM)) {
      getTemp();
      if (temp.Temp != NO_TEMPERATURE) {
        _m1_t(temp);
      } else {
        _m1_d();
      }
    } else {
      _m1_d();
    }
  }
}

/**
 * Show Temprature
 */
void _m1_t(tpElements_t temp) {
  int t1, t2, xo, x, len;

  t1 = temp.Temp;
  t2 = temp.Decimal;

  xo = 5;
      
  len = sprintf(buff, "%d", t1);
  x = ((t1 < 10) && (t1 > -1)) ? 5 : 0;
  if ((t1 < -9) || (t1 > 99)) x = 2;
  dmd.drawString(xo+x, 11, buff, len, GRAPHICS_NORMAL);
  if ((t1 > -10) && (t1 < 100)) {
    dmd.writePixel(xo+10, 15, GRAPHICS_NORMAL, true);
    sprintf(buff, "%d", t2);      
    dmd.drawString(xo+12, 11, buff, 1, GRAPHICS_NORMAL);
  }
     
  x = 17;
  dmd.writePixel(xo+x, 11, GRAPHICS_NORMAL, true);
      
  dmd.writePixel(xo+x+2, 11, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+x+3, 11, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+x+2, 12, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+x+2, 13, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+x+3, 13, GRAPHICS_NORMAL, true);
}

/**
 * Show Date
 */
void _m1_d() {
  int xo, x, len;

  DD = time.Day;
  NN = time.Month;
      
  xo = 4;
  len = sprintf(buff, "%d/%02d", DD, NN);
  x = (DD < 10) ? 3 : 0;
  dmd.drawString(xo+x, 11, buff, len, GRAPHICS_NORMAL);
}

/*
 * Mode 2 show analog clock;
 */
void mode2() {
  int h1, h2, m1, m2, s1, s2, x, xo, yo, len;
  boolean m_c;

  getTime();
  h1 = time.Hour;
  m1 = time.Minute;  if (m1 == 0) m1 = 60;  m1 = round(m1 / 5);
  s1 = time.Second;  if (s1 == 0) s1 = 60;  s1 = round(s1 / 5);
  
  xo = 3;  yo = 0;

  if (Sec == 99) {
    Sec = 100;
    // Face
    dmd.writePixel(xo+7 , yo+7 , GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+7 , yo+1 , GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+10, yo+2 , GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+12, yo+4 , GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+13, yo+7 , GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+12, yo+10, GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+10, yo+12, GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+7 , yo+13, GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+4 , yo+12, GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+2 , yo+10, GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+1 , yo+7 , GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+2 , yo+4 , GRAPHICS_NORMAL, true);  delay(Sec);
    dmd.writePixel(xo+4 , yo+2 , GRAPHICS_NORMAL, true);  delay(Sec);
    Sec = 0;
  }
  
  if (h1 != HH) {
    if (HH == 99) HH = 0;
    h2 = HH;
    HH = h1;
    if (h1 > 11) h1 -= 12;
    if (h2 > 11) h2 -= 12;
    
    _m2_h(xo, yo, h2, false);  // set off for previous time
    _m2_h(xo, yo, h1, true);  /// set on for current time
  }
  
  if (m1 != MM) {
    m2 = MM;
    MM = m1;
    m_c = true;
    
    _m2_m(xo, yo, m2, false);  // set off for previous time
    _m2_m(xo, yo, m1, true);  // set on for current time
  } else {
    m_c = false;
  }
  
  if (s1 != Sec) {
    s2 = Sec;
    Sec = s1;
    
    _m2_s(xo, yo, s2, false);  // set off for previous time
    _m2_s(xo, yo, s1, true);  // set on for current time
  }
  
  if (m_c) {
    DD = time.Day;
    NN = time.Month;
 
    dmd.selectFont(Font4x5);
    xo = 18;  yo = 2;
  
    len = sprintf(buff, "%d", NN);
    x = (len == 1) ? 5 : 2;
    dmd.drawString(xo+x, yo, buff, len, GRAPHICS_NORMAL);
  
    len = sprintf(buff, "%d", DD);
    x = (len == 1) ? 5 : 2;
    dmd.drawString(xo+x, yo+6, buff, len, GRAPHICS_NORMAL);
  }
}

void _m2_s(int xo, int yo, int hHand, byte bPixel) {
  switch (hHand) {
  case 1:  dmd.writePixel(xo+11, yo+1 , GRAPHICS_NORMAL, bPixel);  break;
  case 2:  dmd.writePixel(xo+13, yo+3 , GRAPHICS_NORMAL, bPixel);  break;
  case 3:  dmd.writePixel(xo+14, yo+7 , GRAPHICS_NORMAL, bPixel);  break;
  case 4:  dmd.writePixel(xo+13, yo+11, GRAPHICS_NORMAL, bPixel);  break;
  case 5:  dmd.writePixel(xo+11, yo+13, GRAPHICS_NORMAL, bPixel);  break;
  case 6:  dmd.writePixel(xo+7 , yo+14, GRAPHICS_NORMAL, bPixel);  break;
  case 7:  dmd.writePixel(xo+3 , yo+13, GRAPHICS_NORMAL, bPixel);  break;
  case 8:  dmd.writePixel(xo+1 , yo+11, GRAPHICS_NORMAL, bPixel);  break;
  case 9:  dmd.writePixel(xo   , yo+7 , GRAPHICS_NORMAL, bPixel);  break;
  case 10: dmd.writePixel(xo+1 , yo+3 , GRAPHICS_NORMAL, bPixel);  break;
  case 11: dmd.writePixel(xo+3 , yo+1 , GRAPHICS_NORMAL, bPixel);  break;
  default: dmd.writePixel(xo+7 , yo   , GRAPHICS_NORMAL, bPixel);  break;
  }
}

void _m2_m(int xo, int yo, int hHand, byte bPixel) {
  switch (hHand) {
  case 1:  dmd.writePixel(xo+9 , yo+3 , GRAPHICS_NORMAL, bPixel);  break;
  case 2:  dmd.writePixel(xo+11, yo+5 , GRAPHICS_NORMAL, bPixel);  break;
  case 3:  dmd.writePixel(xo+12, yo+7 , GRAPHICS_NORMAL, bPixel);  break;
  case 4:  dmd.writePixel(xo+11, yo+9 , GRAPHICS_NORMAL, bPixel);  break;
  case 5:  dmd.writePixel(xo+9 , yo+11, GRAPHICS_NORMAL, bPixel);  break;
  case 6:  dmd.writePixel(xo+7 , yo+12, GRAPHICS_NORMAL, bPixel);  break;
  case 7:  dmd.writePixel(xo+5 , yo+11, GRAPHICS_NORMAL, bPixel);  break;
  case 8:  dmd.writePixel(xo+3 , yo+9 , GRAPHICS_NORMAL, bPixel);  break;
  case 9:  dmd.writePixel(xo+2 , yo+7 , GRAPHICS_NORMAL, bPixel);  break;
  case 10: dmd.writePixel(xo+3 , yo+5 , GRAPHICS_NORMAL, bPixel);  break;
  case 11: dmd.writePixel(xo+5 , yo+3 , GRAPHICS_NORMAL, bPixel);  break;
  default: dmd.writePixel(xo+7 , yo+2 , GRAPHICS_NORMAL, bPixel);  break;
  }
}

void _m2_h(int xo, int yo, int hHand, byte bPixel) {
  switch (hHand) {
  case 1:  dmd.writePixel(xo+8 , yo+5 , GRAPHICS_NORMAL, bPixel);  break;
  case 2:  dmd.writePixel(xo+9 , yo+6 , GRAPHICS_NORMAL, bPixel);  break;
  case 3:  dmd.writePixel(xo+9 , yo+7 , GRAPHICS_NORMAL, bPixel);  break;
  case 4:  dmd.writePixel(xo+9 , yo+8 , GRAPHICS_NORMAL, bPixel);  break;
  case 5:  dmd.writePixel(xo+8 , yo+9 , GRAPHICS_NORMAL, bPixel);  break;
  case 6:  dmd.writePixel(xo+7 , yo+9 , GRAPHICS_NORMAL, bPixel);  break;
  case 7:  dmd.writePixel(xo+6 , yo+9 , GRAPHICS_NORMAL, bPixel);  break;
  case 8:  dmd.writePixel(xo+5 , yo+8 , GRAPHICS_NORMAL, bPixel);  break;
  case 9:  dmd.writePixel(xo+5 , yo+7 , GRAPHICS_NORMAL, bPixel);  break;
  case 10: dmd.writePixel(xo+5 , yo+6 , GRAPHICS_NORMAL, bPixel);  break;
  case 11: dmd.writePixel(xo+6 , yo+5 , GRAPHICS_NORMAL, bPixel);  break;
  default: dmd.writePixel(xo+7 , yo+5 , GRAPHICS_NORMAL, bPixel);  break;
  }
}
