/**
 *
 */

#include "App.h"
#include "AppDigital.h"
#include <avr/pgmspace.h>
#include <DMD.h>
#include <Time.h>
#include "font9.h"
#include "font8.h"

const char s_HOUR_MASK[] PROGMEM = "%d";
const char s_MIN_MASK[] PROGMEM = "%02d";
const char s_DATE_MASK[] PROGMEM = "%d/%02d";
const char s_TEMP_MASK[] PROGMEM = "%d.%01d*";  // [*] in font is ['C]

static char buf2[8];

/* AppDigital::AppDigital() {
  // do nothing
} */

void AppDigital::reset() {
  HH = 99;
  MM = 99;
  Sec = 99;
  dmd.clearScreen(true);
}

void AppDigital::loop() {
  byte h1, m1, s1, xo;
  bool m_c;

  h1 = time.Hour;
  m1 = time.Minute;
  s1 = time.Second;

  bool isPM = (h1 > 12);
  if (isPM) h1 -= 12;
  if (h1 == 0) h1 = 12;

  // will be drawing, so set up DMD font  
  if ((m1 != MM) || (h1 != HH)) dmd.selectFont(font9);

  if (h1 != HH) {
    dmd.clearScreen(true);

    dmd.drawFilledBox(0, 11, 1, 12, (isPM ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
    dmd.writePixel(0, 13, (isPM ? GRAPHICS_NORMAL : GRAPHICS_INVERSE), 1);
    dmd.drawFilledBox(30, 12, 32, 13, (isDst ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
    dmd.writePixel(31, 11, (isDst ? GRAPHICS_NORMAL : GRAPHICS_INVERSE), 1);

    xo = (h1 > 9) ? 1 : 4;

    // display hour
    HH = h1;
    sprintf(buff, strcpy_P(buf2, s_HOUR_MASK), h1);
    if (h1 > 9) {
      dmd.drawString(xo, 0, buff, 2, GRAPHICS_NORMAL);
    } else {
      dmd.drawString(xo, 0, buff, 1, GRAPHICS_NORMAL);
    } 
  }

  xo = (h1 > 9) ? 15 : 11;

  m_c = (m1 != MM);
  if (m_c) {
    // display minutes
    MM = m1;
    sprintf(buff, strcpy_P(buf2, s_MIN_MASK), MM);
    dmd.drawString(xo+3, 0, buff, 2, GRAPHICS_NORMAL);
  }
  
  if (s1 != Sec) {
    // display secs (or the flashing ':')
    Sec = s1;
    dmd.drawFilledBox(xo, 2, xo+1, 3, (isOdd(s1) ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
    dmd.drawFilledBox(xo, 5, xo+1, 6, (isOdd(s1) ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
  }

  if (m_c) {
    // clear bottom data area
    dmd.drawFilledBox(2, 9, 29, 15, GRAPHICS_INVERSE);

    if (isOdd(MM)) {
      if (temp.Temp > -41) {  // DS3232SN range is -40'C to +85'C
        doTemp();
      } else {
        doDate();
      }
    } else {
      doDate();
    }
  }
}

/**
 * Show Date
 */
void AppDigital::doDate() {
  byte xo, len;

  byte d1 = time.Day;
  byte m1 = time.Month;  
      
  xo = (d1 < 10) ? 7 : 4;
  len = sprintf(buff, strcpy_P(buf2, s_DATE_MASK), d1, m1);
  dmd.selectFont(font8);
  dmd.drawString(xo, 9, buff, len, GRAPHICS_NORMAL);
}

/**
 * Show Temprature
 */
void AppDigital::doTemp() {
  byte t1, t2, xo, x, len;

  getTemp();
  t1 = temp.Temp;
  t2 = round(temp.Dec2 / 10);

  xo = 5;
  len = sprintf(buff, strcpy_P(buf2, s_TEMP_MASK), t1, t2);
  dmd.selectFont(font8);
  dmd.drawString(xo, 9, buff, len, GRAPHICS_NORMAL);
}

AppDigital DigitalApp = AppDigital();  // instance
