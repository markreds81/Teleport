#include "ZSerial.h"
#include "z/config.h"

ZSerial::ZSerial(int uart_nr) : HardwareSerial(uart_nr)
{
    lastActivity = 0;
}

ZSerial::~ZSerial() {}

void ZSerial::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms)
{
    Base::begin(baud, config, rxPin, txPin, invert, timeout_ms);
    pinMode(PIN_LED_DATA, OUTPUT);
    digitalWrite(PIN_LED_DATA, LOW);
}

int ZSerial::available()
{
    int avail = Base::available();
    if (avail > 0)
    {
        lastActivity = millis();
        digitalWrite(PIN_LED_DATA, HIGH);
    }
    else if ((millis() - lastActivity) > 10)
    {
        digitalWrite(PIN_LED_DATA, LOW);
    }
    return avail;
}

size_t ZSerial::write(uint8_t c)
{
    lastActivity = millis();
    digitalWrite(PIN_LED_DATA, HIGH);
    return Base::write(c);
}

size_t ZSerial::write(const uint8_t *buffer, size_t size)
{
    lastActivity = millis();
    digitalWrite(PIN_LED_DATA, HIGH);
    return Base::write(buffer, size);
}
