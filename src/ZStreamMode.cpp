#include "ZStreamMode.h"
#include "ZDebug.h"
#include "ZModem.h"

ZStreamMode::ZStreamMode(ZModem *m) : ZMode(m)
{
    // NOP
}

static bool isPETSCII() { return false; }

void ZStreamMode::serialIncoming()
{
    int available = modem->serialAvailable();
    if (available == 0)
    {
        return;
    }

    size_t i = 0;
    while (--available)
    {
        uint8_t c = modem->serialRead();
        if ((c == 27 || i > 0) && !isPETSCII())
        {
            escapeBuffer[i++] = c;
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (i >= ZMODE_ESCAPE_MAX_LEN) || (i == 2 && c != '['))
            {
                break;
            }
            if (available == 0)
            {
                delay(5);
                available = modem->serialAvailable();
            }
        }
        if (c == '+' && (escapeCount > 0 || (millis() - escapeTime) > 1000))
        {
            DPRINTLN(escapeCount++);
        }
        else if (c != '+')
        {
            escapeCount = 0;
            escapeTime = millis();
            DPRINTLN(escapeCount);
        }
        if (i == 0)
        {
            modem->socketWrite(c);
        }
    }
    if (i > 0)
    {
        modem->socketWrite(escapeBuffer, i);
    }
    expireTime = (escapeCount == 3) ? millis() + 800 : 0;
}

void ZStreamMode::tick()
{
    if (modem->connected())
    {
        while (modem->socketAvailable() > 0 && modem->serialAvailableForWrite() > 0)
        {
            char c = modem->socketRead();
            modem->serialWrite(c);
        }
        if (expireTime > 0 && (millis() > expireTime))
        {
            expireTime = 0;
            if (escapeCount == 3)
            {
                escapeCount = 0;
                modem->switchBackToCommandMode();
            }
        }
    }
    else
    {
        modem->disconnect();
        modem->switchBackToCommandMode();
    }
}