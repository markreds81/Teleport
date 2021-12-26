#include "ZDataMode.h"

ZDataMode::ZDataMode(ZSerial &serial) : serialPort(serial)
{

}

ZDataMode::~ZDataMode()
{

}

bool ZDataMode::tick()
{
    return false;
}