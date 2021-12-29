#include "ZMode.h"

ZMode::ZMode(ZModem *m) : modem(m)
{

}

ZMode::~ZMode()
{

}

void ZMode::tick()
{
    // NOP
}

void ZMode::serialIncoming()
{
    // NOP
}