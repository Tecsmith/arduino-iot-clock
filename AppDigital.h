/**
 *
 */

#ifndef APPDIGITAL_h
#define APPDIGITAL_h

#include "App.h"

class AppDigital : App {
  public:
    /* AppDigital(); */
    void reset();
    void loop();
  private:
    byte HH, MM, Sec;
    void doDate();
    void doTemp();
};

extern AppDigital DigitalApp;

#endif
