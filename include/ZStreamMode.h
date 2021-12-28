#ifndef ZSTREAM_MODE_H
#define ZSTREAM_MODE_H

#include "ZMode.h"

class ZStreamMode : public ZMode
{
private:
    short escapeCount;
    unsigned long escapeMillis;

public:
    ZStreamMode(ZModem *m);

    void tick();
};

#endif