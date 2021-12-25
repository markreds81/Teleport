#ifndef ZMODEM_H
#define ZMODEM_H

#include "z/config.h"
#include "z/types.h"
#include "ZSettings.h"
#include <Arduino.h>

#define MAX_COMMAND_SIZE 256

const char compile_date[] = __DATE__ " " __TIME__;

class ZModem
{
private:
	HardwareSerial &serialPort;
	ZSettings settings;
	uint8_t buffer[MAX_COMMAND_SIZE];
	size_t buflen;
	unsigned long lastNonPlusTimeMs = 0;
	unsigned long currentExpiresTimeMs = 0;
	char CRLF[4];
    char LFCR[4];
    char LF[2];
    char CR[2];
	char BS;
	char EC;
	char ECS[32];
	bool wifiConnected;
	IPAddress *staticIP = nullptr;
	IPAddress *staticDNS = nullptr;
	IPAddress *staticGW = nullptr;
	IPAddress *staticSN = nullptr;

	char lc(char c);
	void setStaticIPs(IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet);
	bool connectWiFi(const char* ssid, const char* password, IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet);
	bool readSerialStream();
	void clearPlusProgress();
	void showInitMessage();
	void sendResponse(ZResult rc);

	ZResult execCommand();
	ZResult execReset();
	ZResult execInfo(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execWiFi(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execEOLN(int vval, uint8_t *vbuf, int vlen, bool isNumber);

public:
	ZModem(HardwareSerial &serial);
	virtual ~ZModem();

	void factoryReset();

	void begin();
	void tick();
};

#endif