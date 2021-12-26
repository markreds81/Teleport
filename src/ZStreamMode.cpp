#include "ZStreamMode.h"

ZStreamMode::ZStreamMode(ZSerial &serial) : ZDataMode(serial)
{

}

void ZStreamMode::switchTo(WiFiClient *aClient)
{
    client = aClient;
}

bool ZStreamMode::tick()
{
    if (client->connected())
    {
        if (client->available())
        {
            String line = client->readStringUntil('\r');
            serialPort.println(line);
            client->stop();
            delay(500);
            delete client;
            return false;
        }
    }
    if (serialPort.available())
    {
        String line = serialPort.readStringUntil('\r');
        client->println(line);
        client->flush();
    }
    return true;
}