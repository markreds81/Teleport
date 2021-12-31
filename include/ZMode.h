#ifndef ZMODE_H
#define ZMODE_H

#define ZMODE_ESCAPE_MAX_LEN    10

#include "z/types.h"
#include <stdint.h>

class ZModem;   // forward declaration

class ZMode
{
protected:
    ZModem *modem;
    struct ZEscape esc;
public:
    ZMode(ZModem *m);
    virtual ~ZMode();
    
    virtual void tick();
};

#endif