#ifndef LOGGER_H
#define LOGGER_H

#include "main.h"
#include "SPIFFS.h"

extern bool logFileOpen;
extern bool logFileDebug;
extern File logFile;

void logSerialIn(const uint8_t c);
void logSerialOut(const uint8_t c);
void logSocketIn(const uint8_t c);
void logSocketIn(const uint8_t *c, int n);
void logSocketOut(const uint8_t c);
void logPrintf(const char *format, ...);
void logPrintfln(const char* format, ...);
void logPrintln(const char* msg);
char *TOHEX(const char *s, char *hex, const size_t len);
char *TOHEX(long a);
char *TOHEX(int a);
char *TOHEX(unsigned int a);
char *TOHEX(unsigned long a);
char *tohex(uint8_t a);
char *TOHEX(uint8_t a);
uint8_t FROMHEX(uint8_t a1, uint8_t a2);
char *FROMHEX(const char *hex, char *s, const size_t len);

#endif