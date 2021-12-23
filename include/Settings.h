#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

class Settings
{
public:
    static IPAddress *parseIP(const char *str);
};

#endif