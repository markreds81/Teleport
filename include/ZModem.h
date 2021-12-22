#ifndef ZMODEM_H
#define ZMODEM_H

#include "config.h"

#define MAX_COMMAND_SIZE 256

#define ASCII_BS 8
#define ASCII_XON 17
#define ASCII_XOFF 19
#define ASCII_DC4 20
#define ASCII_DELETE 127

const char compile_date[] = __DATE__ " " __TIME__;

#include <Arduino.h>

enum ZResult
{
	ZOK,
	ZERROR,
	ZCONNECT,
	ZNOCARRIER,
	ZNOANSWER,
	ZIGNORE,
	ZIGNORE_SPECIAL
};

enum FlowControlType
{
	FCT_RTSCTS = 0,
	FCT_NORMAL = 1,
	FCT_AUTOOFF = 2,
	FCT_MANUAL = 3,
	FCT_DISABLED = 4,
	FCT_INVALID = 5
};

class ZModem
{
private:
	HardwareSerial &serialPort;
	uint8_t buffer[MAX_COMMAND_SIZE];
	size_t buflen;
	char CRLF[4];
    char LFCR[4];
    char LF[2];
    char CR[2];
	char BS;
	char EC;
	char ECS[32];
	String EOLN;
	bool echoActive;
	bool numericResponses;
	unsigned long lastNonPlusTimeMs = 0;
	unsigned long currentExpiresTimeMs = 0;
	FlowControlType flowControlType;
	bool wifiConnected;
	String wifiSSI;

	char lc(char c);
	void setDefaults();
	bool readSerialStream();
	void clearPlusProgress();
	void showInitMessage();
	void sendResponse(ZResult rc);

	ZResult execSerialCommand();
	ZResult execReset();
	ZResult execInfo(int vval, uint8_t *vbuf, int vlen, bool isNumber);

public:
	ZModem(HardwareSerial &serial);
	virtual ~ZModem();

	void factoryReset();

	void begin();
	void tick();
};

#endif