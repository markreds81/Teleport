#ifndef ZMODE_H
#define ZMODE_H

#define ZMODE_ESCAPE_MAX_LEN    10

#include <stdint.h>

class ZModem;   // forward declaration

class ZMode
{
protected:
    ZModem *modem;
    uint8_t escapeBuffer[ZMODE_ESCAPE_MAX_LEN];
    int escapeCount = 0;
    unsigned long escapeTime = 0;
    unsigned long expireTime = 0;
public:
    ZMode(ZModem *m);
    virtual ~ZMode();
    
    virtual void serialIncoming();
    virtual void tick();
};

#endif