#ifndef CONN_SETTINGS_H
#define CONN_SETTINGS_H

#include "main.h"

enum ConnFlag
{
    FLAG_DISCONNECT_ON_EXIT = 1,
    FLAG_PETSCII = 2,
    FLAG_TELNET = 4,
    FLAG_ECHO = 8,
    FLAG_XONXOFF = 16,
    FLAG_SECURE = 32,
    FLAG_RTSCTS = 64
};

class ConnSettings
{
public:
    boolean petscii = false;
    boolean telnet = false;
    boolean echo = false;
    boolean xonxoff = false;
    boolean rtscts = false;
    boolean secure = false;

    ConnSettings(int flagBitmap);
    ConnSettings(const char *dmodifiers);
    ConnSettings(String modifiers);
    int getBitmap();
    int getBitmap(FlowControlType forceCheck);
    String getFlagString();

    static void IPtoStr(IPAddress *ip, String &str);
    static IPAddress *parseIP(const char *ipStr);
};

#endif