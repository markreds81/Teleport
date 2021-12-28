#ifndef ZMODE_H
#define ZMODE_H

class ZModem;   // forward declaration

class ZMode
{
protected:
    ZModem *modem;
public:
    ZMode(ZModem *m);
    virtual ~ZMode();
    
    virtual void tick();
};

#endif