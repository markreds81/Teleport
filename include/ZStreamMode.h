#ifndef ZSTREAM_MODE_H
#define ZSTREAM_MODE_H

#include "ZMode.h"

class ZStreamMode : public ZMode
{
public:
    ZStreamMode(ZModem *m);

    void serialIncoming();
    void tick();
};

#endif