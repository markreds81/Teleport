#ifndef PROFILE_H
#define PROFILE_H

#include <Arduino.h>
#include "types.h"

class Profile
{
private:
    uint8_t regs[112];

public:
    char hostname[64];
    char wifiSSID[32];
    char wifiPSWD[64];
    int baudRate;

    Profile();
    ~Profile();

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

    inline int speakerVolume()
    {
        return regs[22] & 0x03;
    }

    inline void setSpeakerVolume(int volume)
    {
        regs[22] &= ~0x03;
        regs[22] |= (volume & 0x03);
    }

    inline int speakerControl()
    {
        return (regs[22] >> 2) & 0x03;
    }

    inline void setSpeakerControl(int control)
    {
        regs[22] &= ~0x0C;
        regs[22] |= ((control & 0x03) << 2);
    }
};

#endif