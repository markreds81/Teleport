#ifndef CONFIG_MODE_H
#define CONFIG_MODE_H

#include "main.h"
#include "SerialBuffer.h"
#include "WiFiServerNode.h"

class ZConfig : public ZMode
{
private:
    enum ZConfigMenu
    {
        ZCFGMENU_MAIN = 0,
        ZCFGMENU_NUM = 1,
        ZCFGMENU_ADDRESS = 2,
        ZCFGMENU_OPTIONS = 3,
        ZCFGMENU_WIMENU = 4,
        ZCFGMENU_WIFIPW = 5,
        ZCFGMENU_WICONFIRM = 6,
        ZCFGMENU_FLOW = 7,
        ZCFGMENU_BBSMENU = 8,
        ZCFGMENU_NEWPORT = 9,
        ZCFGMENU_NEWHOST = 10,
        ZCFGMENU_NOTES = 11,
        ZCFGMENU_NETMENU = 12,
        ZCFGMENU_SUBNET = 13
    } currState;

    ZSerial serial; // storage for serial settings only

    void switchBackToCommandMode();
    void doModeCommand();
    bool showMenu;
    bool savedEcho;
    String EOLN;
    const char *EOLNC;
    unsigned long lastNumber;
    String lastAddress;
    String lastOptions;
    String lastNotes;
    WiFiServerSpec serverSpec;
    bool newListen;
    bool useDHCP;
    bool settingsChanged = false;
    char netOpt = ' ';
    int lastNumNetworks = 0;
    IPAddress lastIP;
    IPAddress lastDNS;
    IPAddress lastGW;
    IPAddress lastSN;

public:
    void switchTo();
    void serialIncoming();
    void loop();
};

extern ZConfig configMode;

#endif