/**
 *
 */

#include "AppChange.h"
#include "App.h"
#include <stdint.h>
#include <avr/pgmspace.h>
#include <Time.h>
#include <DMD.h>
#include "RTC.h"
#include "font8.h"
#include "font9.h"

const char s_C_TIME[] PROGMEM = "Time";
const char s_C_DATE[] PROGMEM = "Date";
static const char s_2_MASK[] = "%02d";

void AppChange::key(uint16_t buttonCode, int &mode) {
  // check modes
  byte len;
  tmElements_t _U;

  if ((mode == MODE_DIGITAL) || (mode == MODE_ANALOG) || (mode == MODE_LARGE)) {
    // Change to time mode
    _oldMode = mode;
    mode = MODE_SET_TIME;
    _mode = mode;
    _changing = false;

    dmd.clearScreen(true);
    dmd.selectFont(font8);
    dmd.drawString(6, 0, strcpy_P(buff, s_C_TIME), 4, GRAPHICS_NORMAL);
  } else if (mode == MODE_DATE) {
    // Change to date mode
    _oldMode = mode;
    mode = MODE_SET_DATE;
    _mode = mode;
    _changing = false;

    dmd.clearScreen(true);
    dmd.selectFont(font8);
    dmd.drawString(6, 0, strcpy_P(buff, s_C_DATE), 4, GRAPHICS_NORMAL);
  } else if (mode == MODE_SET_TIME) {
    // Setting time
    if (!_changing) _uct = now();
    breakTime(_uct, _U);
    if (!_changing) {
      _changing = true;
      _U.Second = 0;
    }
    switch (buttonCode) {
      case 11: incMinute(_U); break;
      case 12: decMinute(_U); break;
      case 21: incHour(_U); break;
      case 22: decHour(_U); break;
    }
    getTime( makeTime(_U) );
    showTime();
  } else if (mode == MODE_SET_DATE) {
    // Setting date
    if (!_changing) _uct = now();
    breakTime(_uct, _U);
    if (!_changing) {
      _changing = true;
      _U.Wday = 0;
    }
    switch (buttonCode) {
      case 11: incDay(_U); break;
      case 12: decDay(_U); break;
      case 21: incMonth(_U); break;
      case 22: decMonth(_U); break;
    }
    getTime( makeTime(_U) );
    showDate();
  }
}

void AppChange::loop() {
  if (!_changing) {
    getTime( now() );
    if (_mode == MODE_SET_TIME) {
      showTime();
    } else if (_mode == MODE_SET_DATE) {
      showDate();
    }
  } else {
    // do nothing, key() takes care of display
  }
}

int AppChange::save() {
  if (_changing) {
    tmElements_t _U;  breakTime( _uct, _U );
    tmElements_t _T;  breakTime( now(), _T );
    
    if (_mode == MODE_SET_TIME) {
      _T.Hour = _U.Hour;
      _T.Minute = _U.Minute;
      _T.Second = 0;
    } else if (_mode == MODE_SET_DATE) {
      _T.Year = _U.Year;
      _T.Month = _U.Month;
      _T.Day = _U.Day;
      _T.Wday = 0;
    }
    RTC.write(_T);
    setTime( makeTime(_T) );
  }
  return _oldMode;
}

int AppChange::cancel() {
  _changing = false;
  return _oldMode;
}

void AppChange::getTime() {
  time_t tl = timezone.toLocal( _uct );
  breakTime(tl, _E);
  _E.Year += 70;
}

void AppChange::getTime(time_t set_uct) {
  _uct = set_uct;
  getTime();
}

void AppChange::showTime() {
  byte len;
  len = sprintf(buff, s_2_MASK, _E.Hour);    dmd.drawString(1,  8, buff, len, GRAPHICS_NORMAL);
  len = sprintf(buff, s_2_MASK, _E.Minute);  dmd.drawString(12, 8, buff, len, GRAPHICS_NORMAL);
  len = sprintf(buff, s_2_MASK, _E.Second);  dmd.drawString(23, 8, buff, len, GRAPHICS_NORMAL);
}

void AppChange::showDate() {
  byte len;
  byte y = _E.Year;
  while (y > 100) y -= 100;
  len = sprintf(buff, s_2_MASK, _E.Day);    dmd.drawString(1, 8, buff, len, GRAPHICS_NORMAL);
  len = sprintf(buff, s_2_MASK, _E.Month);  dmd.drawString(12, 8, buff, len, GRAPHICS_NORMAL);
  len = sprintf(buff, s_2_MASK, y);         dmd.drawString(23,  8, buff, len, GRAPHICS_NORMAL);
}

void AppChange::incMinute(tmElements_t &e) {
  e.Minute++;
  if (e.Minute > 59) {
    e.Minute = 0;
    incHour(e);
  }
}

void AppChange::decMinute(tmElements_t &e) {
  if (e.Minute == 0) {
    e.Minute = 59;
    decHour(e);
  } else e.Minute--;
}

void AppChange::incHour(tmElements_t &e) {
  e.Hour++;
  if (e.Hour > 23) e.Hour = 0;
}

void AppChange::decHour(tmElements_t &e) {
  if (e.Hour == 0) e.Hour = 23;
  else e.Hour--;
}

#define LEAP_YEAR(Y) ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

byte daysInMonth(const tmElements_t e) {
  if ((e.Month == 2) && (LEAP_YEAR(e.Year))) return monthDays[e.Month-1] + 1;
  else return monthDays[e.Month-1];
}

void AppChange::incDay(tmElements_t &e) {
  e.Day++;
  if (e.Day > daysInMonth(e)) {
    e.Day = 1;
    incMonth(e);
  }
}

void AppChange::decDay(tmElements_t &e) {
  if (e.Day <= 1) {
    decMonth(e);
    e.Day = daysInMonth(e);
  } else
    e.Day--;
}

void AppChange::incMonth(tmElements_t &e) {
  e.Month++;
  if (e.Month > 12) {
    e.Month = 1;
    e.Year++;
  }
}

void AppChange::decMonth(tmElements_t &e) {
  if (e.Month <= 1) {
    e.Month = 12;
    if (e.Year > 70) e.Year--;
  } else
    e.Month--;
}

AppChange ChangeApp = AppChange();

/* eof */
