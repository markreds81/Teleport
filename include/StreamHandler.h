#ifndef STREAM_HANDLER_H
#define STREAM_HANDLER_H

#include "main.h"
#include "ModeHandler.h"
#include "WiFiClientNode.h"
#include "SerialBuffer.h"

#define ZSTREAM_ESC_BUF_MAX 10

class StreamHandler : public ModeHandler
{
private:
	WiFiClientNode *current = nullptr;
	unsigned long lastNonPlusTimeMs = 0;
	unsigned long currentExpiresTimeMs = 0;
	unsigned long nextFlushMs = 0;
	int plussesInARow = 0;
	ZSerial serial;
	int lastDTR = 0;
	uint8_t escBuf[ZSTREAM_ESC_BUF_MAX];
	unsigned long nextAlarm = millis() + 5000;

	void switchBackToCommandMode(bool logout);
	void socketWrite(uint8_t c);
	void socketWrite(uint8_t *buf, uint8_t len);
	void baudDelay();

	bool isPETSCII();
	bool isEcho();
	FlowControlType getFlowControl();
	bool isTelnet();
	bool isDisconnectedOnStreamExit();

public:
	void switchTo(WiFiClientNode *conn);

	void serialIncoming();
	void loop();
};

extern StreamHandler StreamMode;

#endif