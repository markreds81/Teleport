#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG

#define DPRINT(...)             Serial.print(__VA_ARGS__)
#define DPRINTF(...)            Serial.printf(__VA_ARGS__)
#define DPRINTLN(...)           Serial.println(__VA_ARGS__)

#else

#define DPRINT(...)
#define DPRINTF(...)
#define DPRINTLN(...)

#endif

#endif