#ifndef ZCLIENT_H
#define ZCLIENT_H

#include <WiFiClient.h>

class ZClient : public WiFiClient
{
public:
    ZClient();
};

#endif