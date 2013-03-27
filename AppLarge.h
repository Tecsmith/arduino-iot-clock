/**
 *
 */

#ifndef APPLARGE_h
#define APPLARGE_h

#include "App.h"

class AppLarge : App {
  public:
    /* AppLarge(); */
    void reset();
    void loop();
  private:
    byte HH, MM, Sec;
};

extern AppLarge LargeApp;

#endif
