#include "DebugPort.h"

#if DEBUG_ENABLED

#include "driver/uart.h"

DebugPort Serial(UART_NUM_0);        // global instance

void DebugPort::begin()
{
    HardwareSerial::begin(115200);
    HardwareSerial::setDebugOutput(true);
    HardwareSerial::println("Debug port open and ready.");
}

#endif