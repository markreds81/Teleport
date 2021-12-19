#include "SerialBuffer.h"
#include "Logger.h"
#include "driver/uart.h"
#include "ConnSettings.h"
#include "AsciiUtils.h"

static uint8_t TBUF[SER_WRITE_BUFSIZE];
static char FBUF[256];
static int TBUFhead = 0;
static int TBUFtail = 0;
int serialDelayMs = 0;

void serialDirectWrite(uint8_t c)
{
    SerialDTE.write(c);
    if (serialDelayMs > 0)
    {
        delay(serialDelayMs);
    }
    logSerialOut(c);
}

void hwSerialFlush()
{
    SerialDTE.flush();
}

void serialOutDeque()
{
    while ((TBUFhead != TBUFtail) && ((SER_BUFSIZE - SerialDTE.availableForWrite()) < dequeSize))
    {
        serialDirectWrite(TBUF[TBUFhead]);
        TBUFhead++;
        if (TBUFhead >= SER_WRITE_BUFSIZE)
        {
            TBUFhead = 0;
        }
    }
}

int serialOutBufferBytesRemaining()
{
    if (TBUFtail == TBUFhead)
    {
        return SER_WRITE_BUFSIZE - 1;
    }

    if (TBUFtail > TBUFhead)
    {
        int used = TBUFtail - TBUFhead;
        return SER_WRITE_BUFSIZE - used - 1;
    }

    return TBUFhead - TBUFtail - 1;
}

void enqueSerialOut(uint8_t c)
{
    TBUF[TBUFtail] = c;
    TBUFtail++;
    if (TBUFtail >= SER_WRITE_BUFSIZE)
    {
        TBUFtail = 0;
    }
}

void clearSerialOutBuffer()
{
    TBUFtail = TBUFhead;
}

void ensureSerialBytes(int num)
{
    if (serialOutBufferBytesRemaining() < 1)
    {
        serialOutDeque();
        while (serialOutBufferBytesRemaining() < 1)
        {
            yield();
        }
    }
}

void flushSerial()
{
    while (TBUFtail != TBUFhead)
    {
        serialOutDeque();
        yield();
    }
    hwSerialFlush();
}

ZSerial::ZSerial()
{
}

void ZSerial::setPetsciiMode(bool petscii)
{
    petsciiMode = petscii;
}

bool ZSerial::isPetsciiMode()
{
    return petsciiMode;
}

void ZSerial::setFlowControlType(FlowControlType type)
{
    flowControlType = type;
    if (flowControlType == FCT_RTSCTS)
    {
        uart_set_hw_flow_ctrl(UART_NUM_2, UART_HW_FLOWCTRL_DISABLE, 0);
        uint32_t invertMask = 0;
        uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, /*cts_io_num*/ PIN_CTS);
        // cts is input to me, output to true RS232
        if (ctsActive == HIGH) {
            invertMask = invertMask | UART_INVERSE_CTS;
        }
            
        uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, /*rts_io_num*/ PIN_RTS, UART_PIN_NO_CHANGE);
        digitalWrite(PIN_RTS, rtsActive);
        // rts is output to me, input to true RS232
        if (rtsActive == HIGH) {
            invertMask = invertMask | UART_INVERSE_RTS;
        }
                
        //debugPrintf("invert = %d magic values = %d %d, RTS_HIGH=%d, RTS_LOW=%d HIGHHIGH=%d LOWLOW=%d\n",invertMask,ctsActive,rtsActive, DEFAULT_RTS_HIGH, DEFAULT_RTS_LOW, HIGH, LOW);
        if (invertMask != 0) {
            uart_set_line_inverse(UART_NUM_2, invertMask);
        }
            
        const int CUTOFF = 100;
        uart_set_hw_flow_ctrl(UART_NUM_2, UART_HW_FLOWCTRL_CTS_RTS, CUTOFF);
        SerialDebug.println("Boh? serout.ino");
    }
    else {
        uart_set_hw_flow_ctrl(UART_NUM_2, UART_HW_FLOWCTRL_DISABLE, 0);
    }
}

FlowControlType ZSerial::getFlowControlType()
{
    return flowControlType;
}

void ZSerial::setXON(bool isXON)
{
    XON_STATE = isXON;
}

int ZSerial::getConfigFlagBitmap()
{
    return (isPetsciiMode() ? FLAG_PETSCII : 0) | ((getFlowControlType() == FCT_RTSCTS) ? FLAG_RTSCTS : 0) | ((getFlowControlType() == FCT_NORMAL) ? FLAG_XONXOFF : 0) | ((getFlowControlType() == FCT_AUTOOFF) ? FLAG_XONXOFF : 0) | ((getFlowControlType() == FCT_MANUAL) ? FLAG_XONXOFF : 0);
}

bool ZSerial::isXON()
{
    return XON_STATE;
}

bool ZSerial::isSerialOut()
{
    switch (flowControlType)
    {
    case FCT_RTSCTS:
        return (digitalRead(PIN_CTS) == ctsActive);
    case FCT_NORMAL:
    case FCT_AUTOOFF:
    case FCT_MANUAL:
        break;
    case FCT_DISABLED:
        return true;
    case FCT_INVALID:
        return true;
    }
    return XON_STATE;
}

bool ZSerial::isSerialCancelled()
{
    if (flowControlType == FCT_RTSCTS)
    {
        return (digitalRead(PIN_CTS) == ctsInactive);
    }
    return false;
}

bool ZSerial::isSerialHalted()
{
    return !isSerialOut();
}

void ZSerial::enqueByte(uint8_t c)
{
    if (TBUFtail == TBUFhead)
    {
        switch (flowControlType)
        {
        case FCT_DISABLED:
        case FCT_INVALID:
            if ((SerialDTE.availableForWrite() > 0) && (SerialDTE.available() == 0))
            {
                serialDirectWrite(c);
                return;
            }
            break;
        case FCT_RTSCTS:
            if (isSerialOut())
            {
                serialDirectWrite(c);
                return;
            }
            break;
        case FCT_NORMAL:
        case FCT_AUTOOFF:
        case FCT_MANUAL:
            if ((SerialDTE.availableForWrite() >= SER_BUFSIZE) && (SerialDTE.available() == 0) && (XON_STATE))
            {
                serialDirectWrite(c);
                return;
            }
            break;
        }
    }
    // the car jam of blocked bytes stops HERE
    //debugPrintf("%d\n",serialOutBufferBytesRemaining());
    while (serialOutBufferBytesRemaining() < 1)
    {
        if (!isSerialOut())
            delay(1);
        else
            serialOutDeque();
        yield();
    }
    enqueSerialOut(c);
}

void ZSerial::prints(const char *expr)
{
    if (!petsciiMode)
    {
        for (int i = 0; expr[i] != 0; i++)
        {
            enqueByte(expr[i]);
        }
    }
    else
    {
        for (int i = 0; expr[i] != 0; i++)
        {
            enqueByte(ascToPetcii(expr[i]));
        }
    }
}

void ZSerial::printi(int i)
{
    char buf[12];
    prints(itoa(i, buf, 10));
}

void ZSerial::printd(double f)
{
    char buf[12];
    prints(dtostrf(f, 2, 2, buf));
}

void ZSerial::printc(const char c)
{
    if (!petsciiMode)
        enqueByte(c);
    else
        enqueByte(ascToPetcii(c));
}

void ZSerial::printc(uint8_t c)
{
    if (!petsciiMode)
        enqueByte(c);
    else
        enqueByte(ascToPetcii(c));
}

void ZSerial::printb(uint8_t c)
{
    enqueByte(c);
}

size_t ZSerial::write(uint8_t c)
{
    enqueByte(c);
    return 1;
}

size_t ZSerial::write(uint8_t *buf, int bufSz)
{
    for (int i = 0; i < bufSz; i++)
        enqueByte(buf[i]);
    return bufSz;
}

void ZSerial::prints(String str)
{
    prints(str.c_str());
}

void ZSerial::printf(const char *format, ...)
{
    int ret;
    va_list arglist;
    va_start(arglist, format);
    vsnprintf(FBUF, sizeof(FBUF), format, arglist);
    prints(FBUF);
    va_end(arglist);
}

void ZSerial::flushAlways()
{
    while (TBUFtail != TBUFhead)
    {
        hwSerialFlush();
        serialOutDeque();
        yield();
        delay(1);
    }
    hwSerialFlush();
}

void ZSerial::flush()
{
    while ((TBUFtail != TBUFhead) && (isSerialOut()))
    {
        hwSerialFlush();
        serialOutDeque();
        yield();
        delay(1);
    }
    hwSerialFlush();
}

int ZSerial::availableForWrite()
{
    return serialOutBufferBytesRemaining();
}

char ZSerial::drainForXonXoff()
{
    char ch = '\0';
    while (SerialDTE.available() > 0)
    {
        ch = SerialDTE.read();
        logSerialIn(ch);
        if (ch == 3)
            break;
        switch (flowControlType)
        {
        case FCT_NORMAL:
            if ((!XON_STATE) && (ch == 17))
                XON_STATE = true;
            else if ((XON_STATE) && (ch == 19))
                XON_STATE = false;
            break;
        case FCT_AUTOOFF:
        case FCT_MANUAL:
            if ((!XON_STATE) && (ch == 17))
                XON_STATE = true;
            else
                XON_STATE = false;
            break;
        case FCT_INVALID:
            break;
        case FCT_RTSCTS:
            break;
        }
    }
    return ch;
}

int ZSerial::available()
{
    int avail = SerialDTE.available();
    if (avail == 0)
    {
        if ((TBUFtail != TBUFhead) && isSerialOut())
            serialOutDeque();
    }
    return avail;
}

int ZSerial::read()
{
    int c = SerialDTE.read();
    if (c == -1)
    {
        if ((TBUFtail != TBUFhead) && isSerialOut())
            serialOutDeque();
    }
    else
        logSerialIn(c);
    return c;
}

int ZSerial::peek()
{
    return SerialDTE.peek();
}