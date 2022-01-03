#include "ZClient.h"

int ZClient::nextClientId = 1;

ZClient::ZClient() : WiFiClient()
{
    m_id = nextClientId++;
    m_answered = false;
    flags = 0;
}

int ZClient::connect(const char *host, uint16_t port)
{
    strncpy(m_host, host, sizeof(m_host));
    return Base::connect(host, port);
}