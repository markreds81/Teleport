#ifndef ZDATA_MODE_H
#define ZDATA_MODE_H

#include "ZSerial.h"

class ZDataMode
{
protected:
    ZSerial &serialPort;
public:
    ZDataMode(ZSerial &serial);
    virtual ~ZDataMode();
    
    virtual bool tick();
};

#endif