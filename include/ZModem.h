#ifndef ZMODEM_H
#define ZMODEM_H

#include "z/config.h"
#include "z/types.h"
#include "ZSerial.h"
#include "ZClient.h"
#include "ZSettings.h"
#include "ZMode.h"
#include "ZStreamMode.h"
#include <Arduino.h>

#define MAX_COMMAND_SIZE 256

const char compile_date[] = __DATE__ " " __TIME__;

class ZModem
{
private:
	ZSerial *serial;
	ZClient *socket;
	ZSettings settings;
	ZMode *mode;
	ZStreamMode streamMode;
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
	String lastCommand;
	IPAddress *staticIP = nullptr;
	IPAddress *staticDNS = nullptr;
	IPAddress *staticGW = nullptr;
	IPAddress *staticSN = nullptr;
	
	char lc(char c);
	void switchTo(ZMode *newMode);
	void setStaticIPs(IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet);
	bool connectWiFi(const char* ssid, const char* pswd, IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet);
	bool readSerialStream();
	void clearPlusProgress();
	void showInitMessage();
	void sendResponse(ZResult rc);
	void sendConnectionNotice(int id);

	ZResult execCommand();
	ZResult execReset();
	ZResult execInfo(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execWiFi(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execEOLN(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execBaud(int vval, uint8_t *vbuf, int vlen);
	ZResult execDial(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execConnect(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);

public:
	ZModem(ZSerial *s);
	virtual ~ZModem();

	void switchBackToCommandMode();
	//bool escapeSequence(char c);
	//bool escapeComplete();
	void factoryReset();
	void disconnect();
	void begin();
	void tick();

	inline bool connected()
	{
		return socket != nullptr && socket->connected();
	}

	inline int serialAvailable()
	{
		return serial->available();
	}

	inline int serialAvailableForWrite()
	{
		return serial->availableForWrite();
	}

	inline int serialRead()
	{
		return serial->read();
	}

	inline size_t serialWrite(uint8_t c)
	{
		return serial->write(c);
	}

	inline int socketAvailable()
	{
		return socket != nullptr ? socket->available() : 0;
	}

	inline int socketAvailableForWrite()
	{
		return socket != nullptr ? socket->availableForWrite() : 0;
	}

	inline int socketRead()
	{
		return socket != nullptr ? socket->read() : -1;
	}

	inline size_t socketWrite(uint8_t c)
	{
		return socket != nullptr ? socket->write(c) : 0;
	}

	inline size_t socketWrite(const uint8_t *buf, size_t size)
	{
		return socket != nullptr ? socket->write(buf, size) : 0;
	}
};

#endif