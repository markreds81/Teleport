#ifndef ZMODEM_H
#define ZMODEM_H

#include "z/options.h"
#include "z/types.h"
#include "ZSerial.h"
#include "ZClient.h"
#include "ZBuzzer.h"
#include "ZProfile.h"
#include "ZShell.h"
#include "ZConsole.h"
#include "ZUpdater.h"
#include "ZSettings.h"
#include "ZDebug.h"
#include <Arduino.h>
#include <LinkedList.h>
#include <WebServer.h>
#include <ESPmDNS.h>

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
	ZProfile SREG;
	ZBuzzer buzzer;
	ZClient *socket;
	ZShell shell;
	ZConsole console;
	LinkedList<ZClient *> clients;
	WebServer httpServer;
	ZUpdater httpUpdater;
	uint8_t buffer[MAX_COMMAND_SIZE];
	size_t buflen;
	String termType;
	String lastCommand;
	IPAddress *staticIP = nullptr;
	IPAddress *staticDNS = nullptr;
	IPAddress *staticGW = nullptr;
	IPAddress *staticSN = nullptr;
	unsigned long totalBytesTx = 0;
	unsigned long totalBytesRx = 0;
	unsigned long maxRateTx = 0;
	unsigned long maxRateRx = 0;

	void println();
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
	int activeProfile();
	void setActiveProfile(int num);

	size_t socketWrite(uint8_t c);
	size_t socketWrite(const uint8_t *buf, size_t size);
	uint8_t socketRead(unsigned long tmout);

	ZResult execCommand();
	ZResult execInfo(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execTime(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execWiFi(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execBaud(int vval, uint8_t *vbuf, int vlen);
	ZResult execDial(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execConnect(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execHangup(int vval, uint8_t *vbuf, int vlen, bool isNumber);
	ZResult execPhonebook(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
	ZResult execSRegister(uint8_t *vbuf, int vlen);

	void switchTo(ZMode newMode, ZResult rc = ZIGNORE);

public:
	ZModem();
	virtual ~ZModem();

	void factoryReset();
	void disconnect();
	void begin();

	inline bool connected()
	{
		return socket != nullptr && socket->connected();
	}

	inline void tick()
	{
		static unsigned long rateTimer = millis();
		static unsigned long counterTx = 0;
		static unsigned long counterRx = 0;

		switch (mode)
		{
		case ZCOMMAND_MODE:
			if (Serial2.available() > 0 && readSerialStream())
			{
				ZResult rc = execCommand();
				if (SREG.resultCodeEnabled())
				{
					sendResponse(rc);
				}
			}
			break;
		case ZCONSOLE_MODE:
			if (Serial2.available() > 0 && readSerialStream())
			{
				String line = (char *)buffer;
				line.trim();
				DPRINTLN(line);
				console.exec(line);
				buffer[0] = '\0';
				buflen = 0;
			}
			if (console.done(SREG))
			{
				switchTo(ZCOMMAND_MODE, ZOK);
			}
			break;
		case ZSTREAM_MODE:
			if (socket != nullptr && socket->connected())
			{
				// bridge data from DTE to network
				while (Serial2.available() > 0)
				{
					// Tx stats
					totalBytesTx++;
					// read a char at time and process
					char c = Serial2.read();
					if (c != SREG[2] || (millis() - esc.gt1) < SREG.guardTime() || esc.len >= sizeof(esc.buf))
					{
						if (esc.len)
						{
							socketWrite(esc.buf, esc.len);
							esc.len = 0;
							esc.gt2 = 0;
						}
						socketWrite(c);
						esc.gt1 = millis();
					}
					else
					{
						esc.buf[esc.len++] = c;
						if (esc.len >= 3)
						{
							esc.gt2 = millis();
						}
					}
				}
				// check escape sequence
				if (esc.gt2 && (millis() - esc.gt2) > SREG.guardTime())
				{
					esc.gt2 = 0;
					esc.len = 0;
					switchTo(ZCOMMAND_MODE, ZOK);
				}
				// bridge data from network to DTE
				while (socket->available() > 0 && Serial2.availableForWrite() > 0)
				{
					// RX stats
					totalBytesRx++;
					// read char and process
					char c = socket->read();
					if ((!socket->telnetMode() || processIAC(&c)) && (!socket->petsciiMode() || asc2pet(&c)))
						Serial2.write(c);
					// if incoming data from serial interrupt for process them
					if (Serial2.available() > 0)
						break;
				}
				// update trasnfer rates
				if ((millis() - rateTimer) > 1000)
				{
					unsigned long rate;
					rate = totalBytesTx - counterTx;
					if (rate > maxRateTx)
						maxRateTx = rate;
					rate = totalBytesRx - counterRx;
					if (rate > maxRateRx)
						maxRateRx = rate;
					counterTx = totalBytesTx;
					counterRx = totalBytesRx;
					rateTimer = millis();
				}
			}
			else
			{
				// clean up resources
				for (int i = 0; i < clients.size(); i++)
				{
					if (clients.get(i) == socket)
					{
						clients.remove(i);
						delete socket;
						socket = nullptr;
						break;
					}
				}
				// return to command mode
				switchTo(ZCOMMAND_MODE, ZNOCARRIER);
			}
			break;
		case ZPRINT_MODE:
			break;
		case ZSHELL_MODE:
			if (Serial2.available() > 0 && readSerialStream())
			{
				String line = (char *)buffer;
				line.trim();
				shell.exec(line);
				buffer[0] = '\0';
				buflen = 0;
			}
			if (shell.done())
			{
				switchTo(ZCOMMAND_MODE, ZOK);
			}
			break;
		}

		httpServer.handleClient();
	}

	// inline int serialAvailable()
	// {
	// 	return serial->available();
	// }

	// inline int serialAvailableForWrite()
	// {
	// 	return serial->availableForWrite();
	// }

	// inline int serialRead()
	// {
	// 	return serial->read();
	// }

	// inline size_t serialWrite(uint8_t c)
	// {
	// 	return serial->write(c);
	// }

	// inline size_t serialWrite(const uint8_t *buf, size_t size)
	// {
	// 	return serial->write(buf, size);
	// }

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