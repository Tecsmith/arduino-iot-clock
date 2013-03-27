/**
 *
 */

#include "RTC.h"
#include <Time.h>
#include <Wire.h>

uint8_t tbuff[7];

RTCClass::RTCClass() {
  Wire.begin();
}

bool RTCClass::available() {
  return _r(0x05, 1);
}

void RTCClass::begin() {
  tbuff[0] = 0x00;  // Contril Reg
  tbuff[1] = 0x00;  // Control/Status Reg.
  _w(0x0E, 2);
}

time_t RTCClass::get() {
  tmElements_t tm;

  if (!_r(0x00, 7)) return 0;
  tm.Second = bcd2dec(tbuff[0] & 0x7f);
  tm.Minute = bcd2dec(tbuff[1]);
  tm.Hour = bcd2dec(tbuff[2] & 0x3f);  
  tm.Wday = bcd2dec(tbuff[3]);
  tm.Day = bcd2dec(tbuff[4]);
  tm.Month = bcd2dec(tbuff[5]);
  tm.Year = y2kYearToTm( bcd2dec(tbuff[6]) );

  return makeTime(tm);
}

void RTCClass::set(time_t time) {
  tmElements_t tm;
  breakTime(time, tm);
  write(tm);
}

void RTCClass::write(tmElements_t tm) {
  tbuff[0] = dec2bcd(tm.Second);
  tbuff[1] = dec2bcd(tm.Minute);
  tbuff[2] = dec2bcd(tm.Hour);
  tbuff[3] = dec2bcd(tm.Wday);  
  tbuff[4] = dec2bcd(tm.Day);
  tbuff[5] = dec2bcd(tm.Month);
  while (tm.Year > 100) tm.Year -= 100;
  tbuff[6] = dec2bcd( tmYearToY2k(tm.Year) );
  _w(0x00, 7);
}

void RTCClass::setAlarm2(tmElements_t tm) {
  if (!_r(0x0E, 1)) return;  // Control reg.
  tbuff[0] = (tbuff[0] | 0xF9) & 0x06;  // set INTCN & A2IE on
  _w(0x0E, 1);
}

void RTCClass::resetAlarm2() {
  if (!_r(0x0F, 1)) return;  // Control/Status reg.
  tbuff[0] &= 0xFD;  // set A2F off
  _w(0x0F, 1);
}

void RTCClass::readTemperature(tpElements_t &tmp) {
  if (_r(0x11, 2)) {
    tmp.Temp = tbuff[0];
    tmp.Decimal = (tbuff[0] >> 6) * 25;
  } else {
    tmp.Temp = -99;
    tmp.Decimal = 0;
  }
}

uint8_t RTCClass::dec2bcd(uint8_t num) {
  return (num/10 * 16) + (num % 10);
}

uint8_t RTCClass::bcd2dec(uint8_t num) {
  return (num/16 * 10) + (num % 16);
}

bool RTCClass::_r(const uint8_t addr, const int size) {
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(addr);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, size);
  if (Wire.available()) {
    for (uint8_t i = 0; i < size; i++) tbuff[i] = Wire.read();
    return true;
  }
  return false;
}

void RTCClass::_w(const uint8_t addr, const uint8_t size) {
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(addr);
  for (uint8_t i = 0; i < size; i++) Wire.write(tbuff[i]);
  Wire.endTransmission();
}

RTCClass RTC = RTCClass();

/* oef */
