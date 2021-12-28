#include "ZStreamMode.h"
#include "ZDebug.h"
#include "ZModem.h"

ZStreamMode::ZStreamMode(ZModem *m) : ZMode(m)
{
    escapeCount = 0;
    escapeMillis = 0;
}

void ZStreamMode::tick()
{
    if (modem->connected())
    {
        int available = modem->serialAvailable();
        if (!available && escapeCount == 3 && (millis() - escapeMillis) > 1000)
        {
            modem->switchBackToCommandMode();
            escapeCount = 0;
            return;
        }
        while (available > 0)
        {
            char c = modem->serialRead();
            if (c == '+' && (escapeCount > 0 || (millis() - escapeMillis) > 1000))
            {
                escapeCount++;
            }
            else if (c != '+')
            {
                escapeCount = 0;
                escapeMillis = millis();
            }
            else if (escapeCount == 0)
            {
                modem->socketWrite(c);   
            }
            available = modem->serialAvailable();
        }
        while (modem->socketAvailable() > 0 && modem->serialAvailableForWrite() > 0)
        {
            char c = modem->socketRead();
            modem->serialWrite(c);
        }

        // return ZCONTINUE;
    }
    else
    {
        modem->disconnect();
        modem->switchBackToCommandMode();
    }
}