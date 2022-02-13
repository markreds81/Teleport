#ifndef ZSETTINGS_H
#define ZSETTINGS_H

#include "z/types.h"
#include <Arduino.h>
#include <SPIFFS.h>

#define SETTINGS_FILE_NAME "/settings.txt"

class ZSettings
{
private:
    int scanline(File *file, uint8_t *dst, int size);
    int getvalue(File *file, const char *key, char *value, int size, bool decode = false);
    int putvalue(File *file, const char *key, const char *value, bool encode = false);
    int putvalue(File *file, const char *key, int value);
public:
    String EOLN;
    String hostname;
	String wifiSSID;
	String wifiPSWD;
    int baudRate;
    FlowControlMode flowControlMode;
    bool doEcho;
	bool numericResponses;
	bool suppressResponses;
    bool longResponses;

    static IPAddress *parseIP(const char *str);

    ZSettings();
    virtual ~ZSettings();

    void loadFactoryProfile(long id);
    void loadUserProfile(long id);
    void saveUserProfile(long id);
};

//extern ZSettings Settings;

#endif