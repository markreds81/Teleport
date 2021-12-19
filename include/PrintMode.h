#ifndef PRINT_MODE_H
#define PRINT_MODE_H

#include "main.h"
#include "WiFiClientNode.h"
#include "SerialBuffer.h"
#include <SPIFFS.h>

#define DEFAULT_DELAY_MS					5000

enum PrintPayloadType
{
	PETSCII,
	ASCII,
	RAW
};

class PrintMode : public ZMode
{
private:
	WiFiClientNode *wifiSock = nullptr;
	File tfile;
	Stream *outStream = nullptr;
	unsigned int timeoutDelayMs = DEFAULT_DELAY_MS;
	char *lastPrinterSpec = 0;
	unsigned long currentExpiresTimeMs = 0;
	unsigned long nextFlushMs = 0;
	PrintPayloadType payloadType = PETSCII;
	unsigned long lastNonPlusTimeMs = 0;
	int plussesInARow = 0;
	size_t pdex = 0;
	size_t coldex = 0;
	char pbuf[258];
	ZSerial serial;
	char lastLastC = 0;
	char lastC = 0;
	short jobNum = 0;

	size_t writeStr(char *s);
	size_t writeChunk(char *s, int len);
	void switchBackToCommandMode(bool error);
	ZResult finishSwitchTo(char *hostIp, char *req, int port, bool doSSL);
	void announcePrintJob(const char *hostIp, const int port, const char *req);

public:
	ZResult switchTo(char *vbuf, int vlen, bool petscii);
	ZResult switchToPostScript(char *prefix);
	void setLastPrinterSpec(const char *spec);
	char *getLastPrinterSpec();
	void setTimeoutDelayMs(int ms);
	int getTimeoutDelayMs();

	void serialIncoming();
	void loop();
};

extern PrintMode printMode;

#endif