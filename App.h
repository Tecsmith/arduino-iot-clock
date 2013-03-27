/**
 *
 */

#ifndef APP_h
#define APP_h

#include <Time.h>
#include <TimeZone.h>
#include <DMD.h>
#include "RTC.h"

extern DMD dmd;
extern Timezone timezone;

extern tmElements_t time;
extern bool isDst;
extern tpElements_t temp;
extern char buff[8];

static const byte MODE_MAX      = 4;
static const byte MODE_START    = 1;
static const byte MODE_DIGITAL  = 1;
static const byte MODE_ANALOG   = 2;
static const byte MODE_LARGE    = 3;
static const byte MODE_DATE     = 4;
static const byte MODE_SET_TIME = 101;
static const byte MODE_SET_DATE = 102;

#define isOdd(num) ((num & 1))

class App {
  public:
    virtual void loop();
    virtual void reset();
    static void getTime();
    static void getTemp();
};

#endif
