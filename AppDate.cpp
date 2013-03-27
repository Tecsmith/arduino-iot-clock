/**
 *
 */

#include "App.h"
#include "AppDate.h"
#include <Time.h>
#include "font8.h"

const char s_DATE_MASK[] PROGMEM = "%d/%02d";
const char s_YEAR_MASK[] PROGMEM = "%04d";

/* AppDate::AppDate() {
} */

void AppDate::loop() {
  byte d1, n1, w1, len, xo;
  int y1;

  d1 = time.Day;
  if (d1 != DD) {
    DD = d1;

    n1 = time.Month;
    y1 = time.Year + 1970;
    w1 = time.Wday;

    dmd.selectFont(font8);
    dmd.clearScreen(true);

    len = sprintf(buff, strcpy_P(buff, s_DATE_MASK), d1, n1);
    xo = ((len == 4) ? 3 : 0 );
    dmd.drawString(4+xo, 0, buff, len, GRAPHICS_NORMAL);
    len = sprintf(buff, strcpy_P(buff, s_YEAR_MASK), y1);
    dmd.drawString(6, 8, buff, len, GRAPHICS_NORMAL);
  }
}

void AppDate::reset() {
  DD = 99;
}

AppDate DateApp = AppDate();  // instance

/* eof */
