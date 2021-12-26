#ifndef ZSTREAM_MODE_H
#define ZSTREAM_MODE_H

#include "ZDataMode.h"
#include <WiFiClient.h>

class ZStreamMode : public ZDataMode
{
private:
    WiFiClient *client;
public:
    ZStreamMode(ZSerial &serial);

    void switchTo(WiFiClient *aClient);
    bool tick();
};

#endif