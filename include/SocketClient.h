#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <WiFiClient.h>

#define ZCLIENT_FLAG_PETSCII    0x01
#define ZCLIENT_FLAG_TELNET     0x02

class SocketClient : public WiFiClient
{
private:
    static int nextClientId;
    typedef WiFiClient Base;

    int m_id;
    char m_host[32];
    bool m_answered;
    uint8_t flags;

public:
    SocketClient();

    int connect(const char *host, uint16_t port);
    
    inline int id() { return m_id; }
    inline char *host() { return m_host; }
    inline uint16_t port() { return remotePort(); }
    inline bool answered() { return m_answered; }
    inline bool petsciiMode() { return (flags & ZCLIENT_FLAG_PETSCII) == ZCLIENT_FLAG_PETSCII; }
    inline bool telnetMode() { return (flags & ZCLIENT_FLAG_TELNET) == ZCLIENT_FLAG_TELNET; }
    inline void setPetsciiMode(bool state)
    {
        if (state)
            flags |= ZCLIENT_FLAG_PETSCII;
        else
            flags &= ~ZCLIENT_FLAG_PETSCII;
    }
    inline void setTelnetMode(bool state)
    {
        if (state)
            flags |= ZCLIENT_FLAG_TELNET;
        else
            flags &= ~ZCLIENT_FLAG_TELNET;
    }
};

#endif