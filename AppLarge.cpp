/**
 *
 */

#include "App.h"
#include "AppLarge.h"
#include <DMD.h>
#include <Time.h>
#include "font12.h"

/* AppLarge::AppLarge() {
  // do nothing
} */

void AppLarge::reset() {
  HH = 99;
  MM = 99;
  Sec = 99;
  dmd.clearScreen(true);
}

void AppLarge::loop() {
  byte h1, m1, s1, xo;
  bool m_c;

  h1 = time.Hour;
  m1 = time.Minute;
  s1 = time.Second;

  // will be drawing, so set up DMD font  
  if ((m1 != MM) || (h1 != HH)) dmd.selectFont(font12);

  if (h1 != HH) {
    dmd.clearScreen(true);

    // dmd.drawFilledBox(0, 12, 1, 13, (isAvo ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
    // dmd.drawFilledBox(30, 12, 32, 13, (isDst ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));

    xo = (h1 > 9) ? 1 : 4;

    // display hour
    HH = h1;
    sprintf(buff, "%d", h1);
    if (h1 > 9) {
      dmd.drawString(xo, 2, buff, 2, GRAPHICS_NORMAL);
    } else {
      dmd.drawString(xo, 2, buff, 1, GRAPHICS_NORMAL);
    } 
  }

  xo = (h1 > 9) ? 15 : 11;

  m_c = (m1 != MM);
  if (m_c) {
    // display minutes
    MM = m1;
    sprintf(buff, "%02d", MM);
    dmd.drawString(xo+3, 2, buff, 2, GRAPHICS_NORMAL);
  }
  
  if (s1 != Sec) {
    // display secs (or the flashing ':')
    Sec = s1;
    dmd.drawFilledBox(xo, 5, xo+1, 6, (isOdd(s1) ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
    dmd.drawFilledBox(xo, 9, xo+1, 10, (isOdd(s1) ? GRAPHICS_NORMAL : GRAPHICS_INVERSE));
  }
}

AppLarge LargeApp = AppLarge();  // instance
