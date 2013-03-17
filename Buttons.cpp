
#include <stdint.h>
#include <Arduino.h>
#include "Buttons.h"

ButtonDebouncer::ButtonDebouncer() {
  _lastt = 0;
  _lastv = 0;
  _sample_delay = 100;
  _reader_function = NULL;
}

ButtonDebouncer::ButtonDebouncer(read_callback reader_function, unsigned long sample_delay) {
  ButtonDebouncer();
  _reader_function = reader_function;
  _sample_delay = sample_delay;
}

uint16_t ButtonDebouncer::read() {
  unsigned long t;
  uint16_t value;

  if (!_reader_function) return 0xFFFF;

  t = millis();
  if (t < _lastt) _lastt = 0;  // millis() has overflowed, will occure aprox once in 50 days
  if (t > (_lastt + max(_sample_delay, 100))) {  // don't scan analog lines more than once in 100ms
    _lastt = t;
    value = 0;
    (*_reader_function)(value);
    if (value == _lastv) {
      _lastv = 0;  // reset
      return value;
    } else {
      _lastv = value;
    }
  }
  return 0;
}
