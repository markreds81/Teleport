#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <HardwareSerial.h>
#include <Arduino.h>
#include "types.h"

class SerialPort : public HardwareSerial
{
private:
    unsigned long lastActivity;

public:
    using HardwareSerial::HardwareSerial;   // Inheriting constructors

    void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL);
    int available();
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);

    void setFlowControl(FlowControlMode mode);
};

extern SerialPort Serial2;

#endif