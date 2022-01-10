#include "ZSerial.h"
#include "z/options.h"
#include "driver/uart.h"

ZSerial Serial2(UART_NUM_2);    // global instance

void ZSerial::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms)
{
    HardwareSerial::begin(baud, config, rxPin, txPin, invert, timeout_ms);
    pinMode(PIN_LED_DATA, OUTPUT);
    digitalWrite(PIN_LED_DATA, LOW);
    lastActivity = 0;
}

int ZSerial::available()
{
    int avail = HardwareSerial::available();
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
    return HardwareSerial::write(c);
}

size_t ZSerial::write(const uint8_t *buffer, size_t size)
{
    lastActivity = millis();
    digitalWrite(PIN_LED_DATA, HIGH);
    return HardwareSerial::write(buffer, size);
}
