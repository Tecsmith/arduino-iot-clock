/**
 *
 */

#ifndef APPANALOG_h
#define APPANALOG_h

#include "App.h"

class AppAnalog : App {
  public:
    /* AppAnalog(); */
    void reset();
    void loop();
  private:
    static const byte xo = 3;
    static const byte yo = 0;
    byte HH, M2, M3, S2, S3;
    void hourHand(byte hour, byte bPixel);
    void minuteHand(byte minute, byte bPixel);
    void secondHand(byte second, byte bPixel);
};

extern AppAnalog AnalogApp;

#endif
