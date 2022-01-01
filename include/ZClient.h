#ifndef ZCLIENT_H
#define ZCLIENT_H

#include <WiFiClient.h>

class ZClient : public WiFiClient
{
private:
    static int nextClientId;
    typedef WiFiClient Base;

    int m_id;
    char m_host[32];
    bool m_answered;

public:
    ZClient();

    inline int id() { return m_id; }
    inline char *host() { return m_host; }
    inline uint16_t port() { return remotePort(); }
    inline bool answered() { return m_answered; }

    int connect(const char *host, uint16_t port);
};

#endif