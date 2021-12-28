#ifndef ZCLIENT_H
#define ZCLIENT_H

#include <WiFiClient.h>

class ZClient : public WiFiClient
{
private:
    int m_id;

    static int nextClientId;
public:
    ZClient();

    int id();
};

#endif