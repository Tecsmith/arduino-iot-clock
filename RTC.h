/**
 *
 */

#ifndef RTC_H_
#define RTC_H_

#include <Stdint.h>
#include <Wire.h>
#include <Time.h>

#define RTC_I2C_ADDRESS 0x68

typedef struct  { 
  int8_t Temp; 
  uint8_t Dec2; // 2 decimal digets
} tpElements_t, TempElements, *tpElementsPtr_t;

class RTCClass {
  public:
    RTCClass();
    static bool available();
    static void begin();
    static time_t get();
    static void set(time_t time);
    static void write(tmElements_t tm);
    static void setAlarm2(tmElements_t tm);
    static void resetAlarm2();
    // static void readTemperature(tpElements_t &tmp);
  private:
    static uint8_t dec2bcd(uint8_t num);
    static uint8_t bcd2dec(uint8_t num);
    static bool _r(const uint8_t addr, const int size);
    static void _w(const uint8_t addr, const uint8_t size);
};

extern RTCClass RTC;

#endif