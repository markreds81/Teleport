#include "ZStreamMode.h"
#include "ZDebug.h"
#include "ZModem.h"

ZStreamMode::ZStreamMode(ZModem *m) : ZMode(m)
{
    // NOP
}

void ZStreamMode::tick()
{
    if (modem->connected())
    {
        while (modem->serialAvailable() > 0)
        {
            char c = modem->serialRead();
            if (c != '+' || (millis() - esc.gt1) < 100 || esc.len >= sizeof(esc.buf))
            {
                if (esc.len)
                {
                    modem->socketWrite(esc.buf, esc.len);
                    esc.len = 0;
                    esc.gt2 = 0;
                }
                modem->socketWrite(c);
                esc.gt1 = millis();
            }
            else
            {
                esc.buf[esc.len++] = c;
                if (esc.len >= 3)
                {
                    esc.gt2 = millis();
                }
            }
        }
        while (modem->socketAvailable() > 0)
        {
            char c = modem->socketRead();
            modem->serialWrite(c);
        }
        if (esc.gt2 && (millis() - esc.gt2) > 1000)
        {
            esc.gt2 = 0;
            esc.len = 0;
            modem->switchBackToCommandMode();
        }
    }
    else
    {
        modem->disconnect();
        modem->switchBackToCommandMode();
    }
}