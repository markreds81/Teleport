#ifndef Z_PROFILE_H
#define Z_PROFILE_H

#include <Arduino.h>
#include "z/types.h"

class ZProfile
{
private:
    uint8_t regs[112];

public:
    char hostname[64];
    char wifiSSID[32];
    char wifiPSWD[64];
    int baudRate;

    ZProfile();
    ~ZProfile();

    void loadProfile(int num);
    void saveProfile(int num);

    uint8_t &operator[](int index)
    {
        return regs[index];
    }

    inline long guardTime()
    {
        return regs[12] * 20;   // in fiftieths of a second
    }

    inline char escape()
    {
        return char(regs[2]);
    }

    inline char carriageReturn()
    {
        return char(regs[3]);
    }

    inline char lineFeed()
    {
        return char(regs[4]);
    }

    inline size_t registerSize()
    {
        return sizeof(regs);
    }

    inline bool echoEnabled()
    {
        return (regs[14] & 0x02);
    }

    inline void setEchoEnabled(bool enabled)
    {
        if (enabled)
            regs[14] |= 0x02;
        else
            regs[14] &= ~0x02;
    }

    inline bool resultCodeEnabled()
    {
        return (regs[14] & 0x04) != 0x04;
    }

    inline void setResultCodeEnabled(bool enabled)
    {
        if (enabled)
            regs[14] &= ~0x04;
        else
            regs[14] |= 0x04;
    }

    inline bool resultCodeVerbose()
    {
        return (regs[14] & 0x08);
    }

    inline void setResultCodeVerbose(bool verbose)
    {
        if (verbose)
            regs[14] |= 0x08;
        else
            regs[14] &= ~0x08;
    }

    inline bool resultCodeNumeric()
    {
        return !resultCodeVerbose();
    }

    inline void setResultCodeNumeric(bool numeric)
    {
        setResultCodeVerbose(!numeric);
    }

    inline bool resultCodeExtended()
    {
        return false;
    }

    inline FlowControlMode flowControlMode()
    {
        return FlowControlMode(regs[39] & 0x07);
    }

    inline void setFlowControlMode(FlowControlMode mode)
    {
        regs[39] &= ~0x07;
        regs[39] |= mode;
    }
};

#endif