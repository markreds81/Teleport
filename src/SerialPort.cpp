#include "SerialPort.h"
#include "options.h"
#include "driver/uart.h"

SerialPort Serial2(UART_NUM_2); // global instance

void SerialPort::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms)
{
    uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, PIN_RTS, PIN_CTS);
    HardwareSerial::begin(baud, config, rxPin, txPin, invert, timeout_ms);
    pinMode(PIN_LED_DATA, OUTPUT);
    digitalWrite(PIN_LED_DATA, LOW);
    lastActivity = 0;
}

int SerialPort::available()
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

size_t SerialPort::write(uint8_t c)
{
    lastActivity = millis();
    digitalWrite(PIN_LED_DATA, HIGH);
    return HardwareSerial::write(c);
}

size_t SerialPort::write(const uint8_t *buffer, size_t size)
{
    lastActivity = millis();
    digitalWrite(PIN_LED_DATA, HIGH);
    return HardwareSerial::write(buffer, size);
}

void SerialPort::setFlowControl(FlowControlMode mode)
{
    switch (mode)
    {
    case FCM_DISABLED:
    case FCM_UNUSED1:
    case FCM_UNUSED2:
        uart_set_hw_flow_ctrl(UART_NUM_2, UART_HW_FLOWCTRL_DISABLE, 0);
        uart_set_sw_flow_ctrl(UART_NUM_2, false, 0, 0);
        break;
    case FCM_HARDWARE:
        uart_set_hw_flow_ctrl(UART_NUM_2, UART_HW_FLOWCTRL_CTS_RTS, FLOW_CTRL_RX_THRESH);
        break;
    case FCM_SOFTWARE:
        uart_set_sw_flow_ctrl(UART_NUM_2, true, FLOW_CTRL_RX_THRESH_XON, FLOW_CTRL_RX_THRESH_XOFF);
        break;
    case FCM_BOTH:
        uart_set_hw_flow_ctrl(UART_NUM_2, UART_HW_FLOWCTRL_CTS_RTS, FLOW_CTRL_RX_THRESH);
        uart_set_sw_flow_ctrl(UART_NUM_2, true, FLOW_CTRL_RX_THRESH_XON, FLOW_CTRL_RX_THRESH_XOFF);
        break;
    case FCM_TRANSPARENT:
    case FCM_INVALID:
        break;
    }
}