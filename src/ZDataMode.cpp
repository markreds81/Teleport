#include "ZDataMode.h"

ZDataMode::ZDataMode(ZSerial *s) : serial(s)
{

}

ZDataMode::~ZDataMode()
{

}

ZModeResult ZDataMode::tick()
{
    return ZFINISH;
}