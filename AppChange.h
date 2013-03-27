/**
 *
 */

#ifndef APPCHANGE_H_
#define APPCHANGE_H_

#include <stdint.h>
#include <Time.h>

class AppChange {
  public:
    /* AppChange(); */
    void key(uint16_t buttonCode, int &mode);
    void loop();
    int save();  // returns Old Mode
    int cancel();
  private:
    bool _changing;
    int _oldMode;
    int _mode;
    time_t _uct;
    tmElements_t _E;
    void getTime();
    void getTime(time_t set_uct);
    void showTime();
    void showDate();
    void incMinute(tmElements_t &e);
    void decMinute(tmElements_t &e);
    void incHour(tmElements_t &e);
    void decHour(tmElements_t &e);
    void incDay(tmElements_t &e);
    void decDay(tmElements_t &e);
    void incMonth(tmElements_t &e);
    void decMonth(tmElements_t &e);
};

extern AppChange ChangeApp;

#endif
