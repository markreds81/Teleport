#ifndef SERIAL_BUFFER_H
#define SERIAL_BUFFER_H

#include "main.h"
#include "z_types.h"

#define SER_BUFSIZE 0x7F
#define SER_WRITE_BUFSIZE 4096

void serialDirectWrite(uint8_t c);
void serialOutDeque();
int serialOutBufferBytesRemaining();
void clearSerialOutBuffer();
void flushSerial();
void hwSerialFlush();

extern int serialDelayMs;

class ZSerial : public Stream
{
private:
    bool petsciiMode = false;
    FlowControlType flowControlType = DEFAULT_FCT;
    bool XON_STATE = true;
    void enqueByte(uint8_t c);

public:
    ZSerial();
    void setPetsciiMode(bool petscii);
    bool isPetsciiMode();
    void setFlowControlType(FlowControlType type);
    FlowControlType getFlowControlType();
    void setXON(bool isXON);
    bool isXON();
    bool isSerialOut();
    bool isSerialHalted();
    bool isSerialCancelled();
    bool isPacketOut();
    int getConfigFlagBitmap();

    void prints(String str);
    void prints(const char *expr);
    void printc(const char c);
    void printc(uint8_t c);
    virtual size_t write(uint8_t c);
    size_t write(uint8_t *buf, int bufSz);
    void printb(uint8_t c);
    void printd(double f);
    void printi(int i);
    void printf(const char *format, ...);
    void flush();
    void flushAlways();
    int availableForWrite();
    char drainForXonXoff();

    virtual int available();
    virtual int read();
    virtual int peek();
};

#endif