#include "Logger.h"
#include "AsciiUtils.h"

static char HD[3];
static char HDL[17];

bool logFileOpen = false;
bool logFileDebug= false;
File logFile;

void logSerialIn(const uint8_t c)
{
    // NOP
}

void logSerialOut(const uint8_t c)
{
    // NOP
}

void logSocketIn(const uint8_t c)
{
    // NOP
}

void logSocketIn(const uint8_t *c, int n)
{
    // NOP
}

void logSocketOut(const uint8_t c)
{
    // NOP
}

void logPrintf(const char *format, ...)
{
    // NOP
}

void logPrintfln(const char* format, ...)
{
    // NOP
}

void logPrintln(const char* msg)
{
    // NOP
}

uint8_t FROMHEXDIGIT(uint8_t a1)
{
    a1 = lc(a1);
    if ((a1 >= '0') && (a1 <= '9'))
        return a1 - '0';
    if ((a1 >= 'a') && (a1 <= 'f'))
        return 10 + (a1 - 'a');
    return 0;
}

uint8_t FROMHEX(uint8_t a1, uint8_t a2)
{
    return (FROMHEXDIGIT(a1) * 16) + FROMHEXDIGIT(a2);
}

char *FROMHEX(const char *hex, char *s, const size_t len)
{
    int i = 0;
    for (const char *h = hex; *h != 0 && (*(h + 1) != 0) && (i < len - 1); i++, h += 2)
        s[i] = FROMHEX((uint8_t)*h, (uint8_t) * (h + 1));
    s[i] = 0;
    return s;
}

uint8_t *FROMHEX(uint8_t *s, const size_t len)
{
    int i = 0;
    for (int i = 0; i < len; i += 2)
        s[i / 2] = FROMHEX(s[i], s[i + 1]);
    s[i] = 0;
    return s;
}

char *TOHEX(const char *s, char *hex, const size_t len)
{
    int i = 0;
    for (const char *t = s; *t != 0 && (i < len - 2); i += 2, t++)
    {
        char *x = TOHEX(*t);
        hex[i] = x[0];
        hex[i + 1] = x[1];
    }
    hex[i] = 0;
    return hex;
}

char *TOHEX(uint8_t a)
{
    HD[0] = "0123456789ABCDEF"[(a >> 4) & 0x0f];
    HD[1] = "0123456789ABCDEF"[a & 0x0f];
    HD[2] = 0;
    return HD;
}

char *tohex(uint8_t a)
{
    HD[0] = "0123456789abcdef"[(a >> 4) & 0x0f];
    HD[1] = "0123456789abcdef"[a & 0x0f];
    HD[2] = 0;
    return HD;
}

char *TOHEX(unsigned long a)
{
    for (int i = 7; i >= 0; i--)
    {
        HDL[i] = "0123456789ABCDEF"[a & 0x0f];
        a = a >> 4;
    }
    HDL[8] = 0;
    char *H = HDL;
    if ((strlen(H) > 2) && (strstr(H, "00") == H))
        H += 2;
    return H;
}

char *TOHEX(unsigned int a)
{
    for (int i = 3; i >= 0; i--)
    {
        HDL[i] = "0123456789ABCDEF"[a & 0x0f];
        a = a >> 4;
    }
    HDL[4] = 0;
    char *H = HDL;
    if ((strlen(H) > 2) && (strstr(H, "00") == H))
        H += 2;
    return H;
}

char *TOHEX(int a)
{
    return TOHEX((unsigned int)a);
}

char *TOHEX(long a)
{
    return TOHEX((unsigned long)a);
}
