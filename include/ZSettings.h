#ifndef ZSETTINGS_H
#define ZSETTINGS_H

#include "z/types.h"
#include <Arduino.h>

class ZSettings
{
private:
    void setDefaults();
public:
    String EOLN;
    String hostname;
	String wifiSSI;
	String wifiPSW;
    FlowControlType flowControlType;
    bool doEcho;
	bool numericResponses;
	bool suppressResponses;

    ZSettings();
    virtual ~ZSettings();

    void load();
    void save();

    static IPAddress *parseIP(const char *str);
};

#endif