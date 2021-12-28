#include "ZClient.h"

int ZClient::nextClientId = 0;

ZClient::ZClient() : WiFiClient()
{
    m_id = nextClientId++;
}

int ZClient::id()
{
    return m_id;
}