/**
 *
 */

#ifndef APPDATE_H_
#define APPDATE_H_

#include "App.h"

class AppDate : App {
  public:
    /* AppDate(); */
    void loop();
    void reset();
  private:
    byte DD;
};

extern AppDate DateApp;

#endif
