#include "Settings.h"

IPAddress *Settings::parseIP(const char *str)
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