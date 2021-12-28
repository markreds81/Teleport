#include "ZStreamMode.h"
#include "ZDebug.h"

ZStreamMode::ZStreamMode(ZSerial *serial) : ZDataMode(serial)
{

}

void ZStreamMode::switchTo(ZClient *aClient)
{
    client = aClient;
}

ZModeResult ZStreamMode::tick()
{
    if (client->connected())
    {
        while (serial->available() > 0 && client->availableForWrite() > 0)
        {
            char c = serial->read();

            if (c == '+')
            {
                client->flush();
                client->stop();
                delay(500);
                delete client;
                return ZSUSPEND;
            }

            client->write(c);
        }

        while (client->available() > 0 && serial->availableForWrite() > 0)
        {
            char c = client->read();
            serial->write(c);
        }
        
        return ZCONTINUE;
    }
    
    client->stop();
    delay(500);
    delete client;

    return ZLOGOUT;
}