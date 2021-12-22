#ifndef ZDEBUG_H
#define ZDEBUG_H

#define DEBUG 1

#if DEBUG

#define NO_GLOBAL_SERIAL

#include <Arduino.h>

#define DPRINT(...)             DebugPort.print(__VA_ARGS__)
#define DPRINTF(...)            DebugPort.printf(__VA_ARGS__)
#define DPRINTLN(...)           DebugPort.println(__VA_ARGS__)

class ZDebug : public HardwareSerial
{
private:
    typedef HardwareSerial Base;
public:
    ZDebug(int uart_nr);
    ~ZDebug();

    void begin();
};

extern ZDebug DebugPort;

#else

#define DPRINT(...)
#define DPRINTF(...)
#define DPRINTLN(...)

#endif

#endif