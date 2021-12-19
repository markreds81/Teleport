#ifndef WI_FI_SERVER_NODE_H
#define WI_FI_SERVER_NODE_H

#include "main.h"
#include <WiFiServer.h>
#include <SPIFFS.h>

class WiFiServerSpec
{
public:
    int port;
    int id;
    int flagsBitmap = 0;
    char *delimiters = nullptr;
    char *maskOuts = nullptr;
    char *stateMachine = nullptr;

    WiFiServerSpec();
    WiFiServerSpec(WiFiServerSpec &copy);
    ~WiFiServerSpec();

    WiFiServerSpec &operator=(const WiFiServerSpec &);
};

class WiFiServerNode : public WiFiServerSpec
{
public:
    WiFiServer *server;
    WiFiServerNode *next = nullptr;

    WiFiServerNode(int port, int flagsBitmap);
    bool hasClient();
    ~WiFiServerNode();

    static WiFiServerNode *FindServer(int port);
    static void DestroyAllServers();
    static bool ReadWiFiServer(File &f, WiFiServerSpec &node);
    static void SaveWiFiServers();
    static void RestoreWiFiServers();
};

extern WiFiServerNode *servs;

#endif