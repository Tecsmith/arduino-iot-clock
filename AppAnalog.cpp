/**
 *
 */

#include "App.h"
#include "AppAnalog.h"
#include <avr/pgmspace.h>
#include <DMD.h>
#include <Time.h>
#include "font8.h"

/* AppAnalog::AppAnalog() {
} */

void AppAnalog::reset() {
  dmd.clearScreen(true);

  // Draw clock face
  dmd.writePixel(xo+7 , yo+7 , GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+7 , yo+1 , GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+10, yo+2 , GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+12, yo+4 , GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+13, yo+7 , GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+12, yo+10, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+10, yo+12, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+7 , yo+13, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+4 , yo+12, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+2 , yo+10, GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+1 , yo+7 , GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+2 , yo+4 , GRAPHICS_NORMAL, true);
  dmd.writePixel(xo+4 , yo+2 , GRAPHICS_NORMAL, true);

  HH = 99;
  M2 = 99;
  M3 = 99;
  S2 = 99;
  S3 = 99;
}

void AppAnalog::loop() {
  byte h1, h2, m1, m2, m3, s1, s2, s3, d1, n1, len;
  bool h_c, m_c;

  dmd.selectFont(font8);

  h1 = time.Hour;
  m1 = time.Minute;
  s1 = time.Second;

  h_c = (h1 != HH);
  if (h_c) {
    if (HH == 99) HH = 0;
    h2 = HH;
    HH = h1;
    if (h1 > 11) h1 -= 12;
    if (h2 > 11) h2 -= 12;
    
    hourHand(h2, false);  // set off for previous time
    hourHand(h1, true);  /// set on for current time
  }

  m2 = (m1 == 0) ? 60 : m1;
  m2 = round(m2 / 5);
  if (m2 != M2) {
    m3 = M2;
    M2 = m2;
    
    minuteHand(m3, false);  // set off for previous time
    minuteHand(m2, true);  // set on for current time
  }

  m3 = (m1 % 5);
  if (m3 != M3) {
    M3 = m3;
    if (m3 == 0) dmd.drawLine(xo+1, yo+15, xo+5, yo+15, GRAPHICS_INVERSE);
    else dmd.drawLine(xo+1, yo+15, xo+m3, yo+15, GRAPHICS_NORMAL);

    // len = sprintf(buff, "%02d", m1);
    // dmd.drawString(xo+17, yo, buff, len, GRAPHICS_NORMAL);
  }

  s2 = (s1 == 0) ? 60 : s1;
  s2 = round(s2 / 5);
  if (s2 != S2) {
    s3 = S2;
    S2 = s2;
    
    secondHand(s3, false);  // set off for previous time
    secondHand(s2, true);  // set on for current time
  }

  s3 = (s1 % 5);
  if (s3 != S3) {
    S3 = s3;
    if (s3 == 0) dmd.drawLine(xo+9, yo+15, xo+13, yo+15, GRAPHICS_INVERSE);
    else dmd.drawLine(xo+14-s3, yo+15, xo+13, yo+15, GRAPHICS_NORMAL);

    // len = sprintf(buff, "%02d", s1);
    // dmd.drawString(xo+17, yo+8, buff, len, GRAPHICS_NORMAL);
  }

  // Show Month / Day
  if (h_c) {
    byte x;

    n1 = time.Month;
    d1 = time.Day;
 
    len = sprintf(buff, "%d", n1);
    x = (len == 1) ? 3 : 0;
    dmd.drawString(xo+17+x, yo, buff, len, GRAPHICS_NORMAL);
  
    len = sprintf(buff, "%d", d1);
    x = (len == 1) ? 3 : 0;
    dmd.drawString(xo+17+x, yo+8, buff, len, GRAPHICS_NORMAL);
  }
}

void AppAnalog::hourHand(byte hour, byte bPixel) {
  switch (hour) {
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

void AppAnalog::minuteHand(byte minute, byte bPixel) {
  switch (minute) {
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

void AppAnalog::secondHand(byte second, byte bPixel) {
  switch (second) {
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

AppAnalog AnalogApp = AppAnalog();  // instance


/*
 * Mode 2 show analog clock;
 */
/* void mode2() {
}  /* */
