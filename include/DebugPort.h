#ifndef DEBUG_PORT_H
#define DEBUG_PORT_H

#define DEBUG_ENABLED 1

#if DEBUG_ENABLED

#include <HardwareSerial.h>

#define DPRINT(...)             Serial.print(__VA_ARGS__)
#define DPRINTF(...)            Serial.printf(__VA_ARGS__)
#define DPRINTLN(...)           Serial.println(__VA_ARGS__)

class DebugPort : public HardwareSerial
{
public:
    using HardwareSerial::HardwareSerial;   // Inheriting constructors

    void begin();
};

extern DebugPort Serial;

#else

#define DPRINT(...)
#define DPRINTF(...)
#define DPRINTLN(...)

#endif

#endif