#include "ZModem.h"

ZModem::ZModem()
{
    // NOP
}

ZModem::~ZModem()
{
    // NOP
}

int ZModem::checkOpenConnections()
{
    return 0;
}

void ZModem::switchTo(ModeHandler *mode)
{
    currentMode->leaveOut();
    currentMode = mode;
    currentMode->switchTo();
}

void ZModem::begin()
{
    
}

void ZModem::loop()
{

}