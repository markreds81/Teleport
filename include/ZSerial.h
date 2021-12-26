#ifndef ZSERIAL_H
#define ZSERIAL_H

#define NO_GLOBAL_SERIAL

#include <Arduino.h>

class ZSerial : public HardwareSerial
{
private:
    typedef HardwareSerial Base;

    unsigned long lastActivity;
public:
    ZSerial(int uart_nr);
    ~ZSerial();

    void begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL);
    int available();
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);
};

#endif