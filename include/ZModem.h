#ifndef ZMODEM_H
#define ZMODEM_H

#include "z_types.h"
#include "ModeHandler.h"

class ZModem
{
private:
    ModeHandler *currentMode;
    BaudState baudState;
public:
    ZModem();
    virtual ~ZModem();

    int checkOpenConnections();

    void switchTo(ModeHandler *mode);

    void begin();
    void loop();
};

#endif