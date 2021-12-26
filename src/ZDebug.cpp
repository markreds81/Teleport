#include "ZDebug.h"

#if DEBUG

ZDebug::ZDebug(int uart_nr) : HardwareSerial(uart_nr) {}

ZDebug::~ZDebug() {}

void ZDebug::begin()
{
    Base::begin(115200);
    Base::setDebugOutput(true);
    Base::println("Debug port open and ready.");
}

ZDebug DebugPort(0);

#endif