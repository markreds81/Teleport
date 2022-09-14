#include "SocketClient.h"

int SocketClient::nextClientId = 1;

SocketClient::SocketClient() : WiFiClient()
{
    m_id = nextClientId++;
    m_answered = false;
    flags = 0;
}

int SocketClient::connect(const char *host, uint16_t port)
{
    strncpy(m_host, host, sizeof(m_host));
    return Base::connect(host, port);
}