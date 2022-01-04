#ifndef ZMODEM_H
#define ZMODEM_H

#include "z/config.h"
#include "z/types.h"
#include "ZSerial.h"
#include "ZClient.h"
#include "ZSettings.h"
#include "ZPhonebook.h"
#include <Arduino.h>
#include <LinkedList.h>

#define MAX_COMMAND_SIZE 256

const char compile_date[] = __DATE__ " " __TIME__;

class ZModem
{
private:
	static const char *const RESULT_CODES_V0[];
	static const char *const RESULT_CODES_V1[];
	static const unsigned char PET2ASC_TABLE[256];
	static const unsigned char ASC2PET_TABLE[256];

	ZMode mode;
	ZEscape esc;
	ZSerial *serial;
	ZClient *socket;
	ZSettings settings;
	ZPhonebook phonebook;
	LinkedList<ZClient *> clients;
	uint8_t buffer[MAX_COMMAND_SIZE];
	size_t buflen;
	char CRLF[4];
	char LFCR[4];
	char LF[2];
	char CR[2];
	char BS;
	char EC;
	char ECS[32];
	String termType;
	String lastCommand;
	IPAddress *staticIP = nullptr;
	IPAddress *staticDNS = nullptr;
	IPAddress *staticGW = nullptr;
	IPAddress *staticSN = nullptr;

	char lc(char c);
	bool asc2pet(char *c);
	bool processIAC(char *c);
	int modifierCompare(const char *ma, const char *m2);
	void setStaticIPs(IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet);
	bool connectWiFi(const char *ssid, const char *pswd, IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet);
	bool readSerialStream();
	void showInitMessage();
	void sendResponse(ZResult rc);
	void sendConnectionNotice(int id);

	size_t socketWrite(uint8_t c);
	size_t socketWrite(const uint8_t *buf, size_t size);
	uint8_t socketRead(unsigned long tmout);

	ZResult execCommand();
	ZResult execReset();
	ZResult execInfo(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execTime(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execWiFi(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execEOLN(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execBaud(int vval, uint8_t *vbuf, int vlen);
	ZResult execDial(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execConnect(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execHangup(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execPhonebook(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);

	void switchTo(ZMode newMode, ZResult rc = ZIGNORE);
	void commandModeHandler();
	void configModeHandler();
	void streamModeHandler();
	void printModeHandler();

public:
	ZModem(ZSerial *s);
	virtual ~ZModem();

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

	inline size_t serialWrite(const uint8_t *buf, size_t size)
	{
		return serial->write(buf, size);
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
};

#endif