#ifndef ZDATA_MODE_H
#define ZDATA_MODE_H

#include "ZSerial.h"
#include "z/types.h"

class ZDataMode
{
protected:
    ZSerial *serial;
public:
    ZDataMode(ZSerial *s);
    virtual ~ZDataMode();
    
    virtual ZModeResult tick();
};

#endif