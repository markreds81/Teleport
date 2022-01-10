#ifndef ZDEBUG_H
#define ZDEBUG_H

#define DEBUG 1

#if DEBUG

#include <HardwareSerial.h>

#define DPRINT(...)             Serial.print(__VA_ARGS__)
#define DPRINTF(...)            Serial.printf(__VA_ARGS__)
#define DPRINTLN(...)           Serial.println(__VA_ARGS__)

class ZDebug : public HardwareSerial
{
public:
    using HardwareSerial::HardwareSerial;   // Inheriting constructors

    void begin();
};

extern ZDebug Serial;

#else

#define DPRINT(...)
#define DPRINTF(...)
#define DPRINTLN(...)

#endif

#endif