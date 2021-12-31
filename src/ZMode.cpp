#include "ZMode.h"

#include <string.h>

ZMode::ZMode(ZModem *m) : modem(m)
{
    memset(&esc, 0, sizeof(esc));
}

ZMode::~ZMode()
{

}

void ZMode::tick()
{
    // NOP
}