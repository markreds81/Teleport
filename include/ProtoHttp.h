#ifndef PROTO_HTTP_H
#define PROTO_HTTP_H

#include "main.h"
#include <WiFiClient.h>
#include <SPIFFS.h>

bool parseWebUrl(uint8_t *vbuf, char **hostIp, char **req, int *port, bool *doSSL);
bool doWebGetBytes(const char *hostIp, int port, const char *req, const bool doSSL, uint8_t *buf, int *bufSize);
WiFiClient *doWebGetStream(const char *hostIp, int port, const char *req, bool doSSL, uint32_t *responseSize);
bool doWebGet(const char *hostIp, int port, FS *fs, const char *filename, const char *req, const bool doSSL);

#endif