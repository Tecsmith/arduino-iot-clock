
#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <stdint.h>
#include <Arduino.h>

extern "C" {
  typedef void (*read_callback)(uint16_t &);
}

class ButtonDebouncer {
  public:
    ButtonDebouncer();
    ButtonDebouncer(read_callback reader_function, unsigned long sample_delay = 100);
    uint16_t read();
  private:
    read_callback _reader_function;
    unsigned long _sample_delay;
    unsigned long _lastt;
    uint16_t _lastv;
};

#endif
