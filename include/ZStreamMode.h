#ifndef ZSTREAM_MODE_H
#define ZSTREAM_MODE_H

#include "ZDataMode.h"
#include "ZClient.h"

class ZStreamMode : public ZDataMode
{
private:
    ZClient *client;
public:
    ZStreamMode(ZSerial *serial);

    void switchTo(ZClient *aClient);
    ZModeResult tick();
};

#endif