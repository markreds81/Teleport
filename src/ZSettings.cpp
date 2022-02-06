#include "ZSettings.h"
#include "ZDebug.h"
#include "ZBase64.h"
#include "z/version.h"

ZSettings Settings;     // default instance

IPAddress *ZSettings::parseIP(const char *str)
{
    uint8_t dots[4];
    int dotDex = 0;
    char *le = (char *)str;
    const char *ld = str + strlen(str);
    
    if (strlen(str) < 7)
    {
        return NULL;
    }
        
    for (char *e = le; e <= ld; e++)
    {
        if ((*e == '.') || (e == ld))
        {
            if (le == e)
            {
                break;
            }                
            *e = 0;
            String sdot = le;
            sdot.trim();
            if ((sdot.length() == 0) || (dotDex > 3))
            {
                dotDex = 99;
                break;
            }
            dots[dotDex++] = (uint8_t)atoi(sdot.c_str());
            if (e == ld)
                le = e;
            else
                le = e + 1;
        }
    }
    if (dotDex != 4 || *le != 0)
    {
        return nullptr;
    }
        
    return new IPAddress(dots[0], dots[1], dots[2], dots[3]);
}

ZSettings::ZSettings()
{
    doEcho = true;
    numericResponses = false;
    suppressResponses = false;
    longResponses = false;
    baudRate = DEFAULT_BAUD_RATE;
    flowControlMode = FCM_DISABLED;
    EOLN = "\r\n";
    hostname = "";
    wifiSSID = "";
    wifiPSWD = "";
}

ZSettings::~ZSettings()
{
    // NOP
}

int ZSettings::scanline(File *file, uint8_t *dst, int size)
{
    int i = 0;
    while (file->read(&dst[i], 1) > 0 && dst[i] != '\n' && i < size)
    {
        i++;
    }
    dst[i] = '\0';
    return i;
}

int ZSettings::getvalue(File *file, const char *key, char *value, int size, bool decode)
{
    size_t len;
    uint8_t buf[128];

    value[0] = '\0';
    file->seek(0);
    while ((len = scanline(file, buf, sizeof(buf))) > 0)
    {
        if (buf[0] != '#' && strstr((const char *)buf, key) != NULL)
        {
            int i = strlen(key);
            int j = 0;
            while (buf[i] != '=' && i < len)
            {
                i++;
            }
            i++;
            while (buf[i] == ' ' && i < len)
            {
                i++;
            }
            if (decode)
            {
                char *s = (char *)buf + i;
                while (buf[i] != '\r' && buf[i] != '\n' && i < len)
                {
                    i++;
                }
                buf[i] = '\0';  // terminate before deconding
                j = ZBase64::decodeLength(s);
                ZBase64::decode(s, (uint8_t *)value);
            }
            else
            {
                while (buf[i] != '\r' && buf[i] != '\n' && i < len && j < size)
                {
                    value[j++] = buf[i++];
                }
            }
            value[j] = '\0';    // terminate the final string value
            return j;
        }
    }
    return 0;
}

int ZSettings::putvalue(File *file, const char *key, const char *value, bool encode)
{
    int bytesWritten = 0;

    bytesWritten += file->print(key);
    bytesWritten += file->print(" = ");
    if (encode)
    {
        size_t len = strlen(value);
        char encoded[ZBase64::encodeLength(len)];
        ZBase64::encode((const uint8_t *)value, len, encoded);
        bytesWritten += file->println(encoded);
    }
    else
    {
        bytesWritten += file->println(value);
    }
    
    return bytesWritten;
}

int ZSettings::putvalue(File *file, const char *key, int value)
{
    int bytesWritten = 0;

    bytesWritten += file->print(key);
    bytesWritten += file->print(" = ");
    bytesWritten += file->println(value);

    return bytesWritten;
}

void ZSettings::loadFactoryProfile(long id)
{
    switch (id)
    {
    default:
        doEcho = true;
        numericResponses = false;
        suppressResponses = false;
        longResponses = false;
        baudRate = DEFAULT_BAUD_RATE;
        flowControlMode = FCM_DISABLED;
        EOLN = "\r\n";
        hostname = "";
        wifiSSID = "";
        wifiPSWD = "";  
    }

    DPRINTF("%s profile %lu %s\n", "Factory", id, "loaded");
}

void ZSettings::loadUserProfile(long id)
{
    int len;
    char buf[128];

    File file = SPIFFS.open(SETTINGS_FILE_NAME, "r");
    if (file)
    {
        if ((len = getvalue(&file, "EOLN", buf, sizeof(buf), true)) > 0)
        {
            DPRINT("EOLN:");
            for (int i = 0; i < len; i++)
                DPRINTF(" 0x%02X", buf[i]);
            DPRINTLN();
            EOLN = String(buf);
        }

        if (getvalue(&file, "hostname", buf, sizeof(buf)))
        {
            hostname = String(buf);
        }

        if (getvalue(&file, "wifiSSID", buf, sizeof(buf)))
        {
            wifiSSID = String(buf);
        }

        if (getvalue(&file, "wifiPSWD", buf, sizeof(buf)))
        {
            wifiPSWD = String(buf);
        }

        if (getvalue(&file, "baudRate", buf, sizeof(buf)))
        {
            baudRate = atoi(buf);
        }

        if (getvalue(&file, "doEcho", buf, sizeof(buf)))
        {
            doEcho = atoi(buf) != 0 ? 1 : 0;
        }

        if (getvalue(&file, "numericResponses", buf, sizeof(buf)))
        {
            numericResponses = atoi(buf) != 0 ? 1 : 0;
        }

        if (getvalue(&file, "suppressResponses", buf, sizeof(buf)))
        {
            suppressResponses = atoi(buf) != 0 ? 1 : 0;
        }

        if (getvalue(&file, "longResponses", buf, sizeof(buf)))
        {
            longResponses = atoi(buf) != 0 ? 1 : 0;
        }

        file.close();

        DPRINTF("%s profile %lu %s\n", "User", id, "loaded");
    }
    else
    {
        DPRINTF("Unable to %s config file\n", "read");
    }
}

void ZSettings::saveUserProfile(long id)
{
    File file = SPIFFS.open(SETTINGS_FILE_NAME, "w");
    int bytes = 0;

    bytes += putvalue(&file, "version", ZMODEM_VERSION);
    bytes += putvalue(&file, "EOLN", EOLN.c_str(), true);
    bytes += putvalue(&file, "hostname", hostname.c_str());
    bytes += putvalue(&file, "wifiSSID", wifiSSID.c_str());
    bytes += putvalue(&file, "wifiPSWD", wifiPSWD.c_str());
    bytes += putvalue(&file, "baudRate", baudRate);
    bytes += putvalue(&file, "doEcho", doEcho ? 1 : 0);
    bytes += putvalue(&file, "numericResponses", numericResponses ? 1 : 0);
    bytes += putvalue(&file, "suppressResponses", suppressResponses ? 1 : 0);
    bytes += putvalue(&file, "longResponses", longResponses ? 1 : 0);
    
    file.close();

    if (bytes > 0)
    {
        DPRINTF("%s profile %lu %s\n", "User", id, "updated");
    }
    else
    {
        DPRINTF("Unable to %s config file\n", "write");
    }
}
