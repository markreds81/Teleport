#include "ZDebug.h"

#if DEBUG

#include "driver/uart.h"

ZDebug Serial(UART_NUM_0);        // global instance

void ZDebug::begin()
{
    HardwareSerial::begin(115200);
    HardwareSerial::setDebugOutput(true);
    HardwareSerial::println("Debug port open and ready.");
}

#endif