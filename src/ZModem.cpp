#include "ZModem.h"
#include "ZDebug.h"
#include "ZClient.h"
#include "ZSettings.h"
#include <WiFi.h>
#include <SPIFFS.h>

#define TELNET_BINARY 0
#define TELNET_ECHO 1
#define TELNET_LOGOUT 18
#define TELNET_SUPRESS_GO_AHEAD 3
#define TELNET_TERMTYPE 24
#define TELNET_NAWS 31
#define TELNET_TOGGLE_FLOW_CONTROL 33
#define TELNET_LINEMODE 34
#define TELNET_MSDP 69
#define TELNET_MSSP 70
#define TELNET_COMPRESS 85
#define TELNET_COMPRESS2 86
#define TELNET_MSP 90
#define TELNET_MXP 91
#define TELNET_AARD 102
#define TELNET_ATCP 200
#define TELNET_GMCP 201
#define TELNET_SE 240
#define TELNET_AYT 246
#define TELNET_EC 247
#define TELNET_GA 249
#define TELNET_SB 250
#define TELNET_WILL 251
#define TELNET_WONT 252
#define TELNET_DO 253
#define TELNET_DONT 254
#define TELNET_NOP 241
#define TELNET_IAC 255

const char *const ZModem::RESULT_CODES_V0[] = {
	"0", "1", "2", "3", "4", "6", "7", "8"};

const char *const ZModem::RESULT_CODES_V1[] = {
	"OK",
	"CONNECT",
	"RING",
	"NO CARRIER",
	"ERROR",
	"NO DIALTONE",
	"BUSY",
	"NO ANSWER"};

const unsigned char ZModem::PET2ASC_TABLE[256] PROGMEM = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x14, 0x09, 0x0d, 0x11, 0x93, 0x0a, 0x0e, 0x0f,
	0x10, 0x0b, 0x12, 0x13, 0x08, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x0c, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf};

const unsigned char ZModem::ASC2PET_TABLE[256] PROGMEM = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x14, 0x20, 0x0a, 0x11, 0x93, 0x0d, 0x0e, 0x0f,
	0x10, 0x0b, 0x12, 0x13, 0x08, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0xc0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x0c, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

ZModem::ZModem(ZSerial *s) : serial(s)
{
	mode = ZCOMMAND_MODE;
	buffer[0] = '\0';
	buflen = 0;
	BS = ASCII_BS;
	EC = '+';
	termType = DEFAULT_TERMTYPE;
	lastCommand = "";
	strcpy(CRLF, "\r\n");
	strcpy(LFCR, "\n\r");
	strcpy(LF, "\n");
	strcpy(CR, "\r");
	memset(ECS, EC, 3);
	memset(&esc, 0, sizeof(esc));
}

ZModem::~ZModem()
{
}

char ZModem::lc(char c)
{
	if ((c >= 65) && (c <= 90))
	{
		return c + 32;
	}
	if ((c >= 193) && (c <= 218))
	{
		return c - 96;
	}
	return c;
}

bool ZModem::asc2pet(char *c)
{
	return true;
}

bool ZModem::processIAC(char *c)
{
	if (*c == 0xFF)
	{
		*c = socketRead(250);
		if (*c == TELNET_IAC)
		{
			*c = 0xFF;
			return true;
		}
		if (*c == TELNET_WILL)
		{
			char what = socketRead(250);
			uint8_t iacDont[] = {TELNET_IAC, TELNET_DONT, what};
			if (what == TELNET_TERMTYPE)
				iacDont[1] = TELNET_DO;
			socket->write(iacDont, 3);
			return false;
		}
		if (*c == TELNET_DONT)
		{
			char what = socketRead(250);
			uint8_t iacWont[] = {TELNET_IAC, TELNET_WONT, what};
			socket->write(iacWont, 3);
			return false;
		}
		if (*c == TELNET_WONT)
		{
			socketRead(250); // skip
			return false;
		}
		if (*c == TELNET_DO)
		{
			char what = socketRead(250);
			uint8_t iacWont[] = {TELNET_IAC, TELNET_WONT, what};
			if (what == TELNET_TERMTYPE)
				iacWont[1] = TELNET_WILL;
			socket->write(iacWont, 3);
			return false;
		}
		if (*c == TELNET_SB)
		{
			char what = socketRead(250);
			char lastC = *c;
			while (((lastC != TELNET_IAC) || (*c != TELNET_SE)) && (*c >= 0))
			{
				lastC = *c;
				*c = socketRead(250);
			}
			if (what == TELNET_TERMTYPE)
			{
				int len = termType.length() + 6;
				uint8_t buf[len];
				buf[0] = TELNET_IAC;
				buf[1] = TELNET_SB;
				buf[2] = TELNET_TERMTYPE;
				buf[3] = (uint8_t)0;
				sprintf((char *)buf + 4, termType.c_str());
				buf[len - 2] = TELNET_IAC;
				buf[len - 1] = TELNET_SE;
				socket->write(buf, len);
				return false;
			}
		}
		return false;
	}
	return true;
}

int ZModem::modifierCompare(const char *m1, const char *m2)
{
	size_t l1 = strlen(m1);
	size_t l2 = strlen(m2);
	if (l1 != l2)
		return -1;
	for (int i1 = 0; i1 < l1; i1++)
	{
		char c1 = tolower(m1[i1]);
		bool found = false;
		for (int i2 = 0; i2 < l2; i2++)
		{
			char c2 = tolower(m2[i2]);
			found = found || (c1 == c2);
		}
		if (!found)
			return -1;
	}
	return 0;
}

void ZModem::setStaticIPs(IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet)
{
	if (staticIP != nullptr)
	{
		free(staticIP);
	}
	if (staticDNS != nullptr)
	{
		free(staticDNS);
	}
	if (staticGW != nullptr)
	{
		free(staticGW);
	}
	if (staticSN != nullptr)
	{
		free(staticSN);
	}
	staticIP = ip;
	staticDNS = dns;
	staticGW = gateway;
	staticSN = subnet;
}

bool ZModem::connectWiFi(const char *ssid, const char *pswd, IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet)
{
	while (WiFi.status() == WL_CONNECTED)
	{
		WiFi.disconnect();
		delay(100);
		yield();
	}
	digitalWrite(PIN_LED_WIFI, LOW);
	WiFi.mode(WIFI_STA);
	if (ip != NULL && dns != NULL && gateway != NULL && subnet != NULL)
	{
		if (!WiFi.config(*ip, *gateway, *subnet, *dns))
		{
			return false;
		}
	}
	DPRINTF("Connecting to %s ", ssid);
	WiFi.begin(ssid, pswd);
	int attemps = 0;
	while (attemps < 30)
	{
		if (WiFi.status() == WL_CONNECTED && strcmp(WiFi.localIP().toString().c_str(), "0.0.0.0") != 0)
		{
			DPRINTLN("OK");
			if (settings.hostname.length() > 0)
			{
				tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, settings.hostname.c_str());
				WiFi.hostname(settings.hostname);
			}
			digitalWrite(PIN_LED_WIFI, HIGH);
			return true;
		}
		else
		{
			digitalWrite(PIN_LED_WIFI, attemps % 2);
			DPRINT(".");
			delay(500);
		}
		attemps++;
	}
	digitalWrite(PIN_LED_WIFI, LOW);
	DPRINTLN("failed");
	WiFi.disconnect();
	return false;
}

bool ZModem::readSerialStream()
{
	bool crReceived = false;
	while (serial->available() > 0 && !crReceived)
	{
		uint8_t c = serial->read();
		if (c == '\n' || c == '\r')
		{
			if (settings.doEcho)
			{
				serial->write(c);
			}
			crReceived = true;
			break;
		}

		if (c > 0)
		{
			if ((c == ASCII_XOFF) && (settings.flowControlType == FCT_NORMAL))
			{
				DPRINTLN("serial->setXON(false)");
			}
			else if ((c == ASCII_XOFF) && ((settings.flowControlType == FCT_AUTOOFF) || (settings.flowControlType == FCT_MANUAL)))
			{
				DPRINTLN("packetXOn = false");
			}
			else if ((c == ASCII_XON) && (settings.flowControlType == FCT_NORMAL))
			{
				DPRINTLN("serial->setXON(true)");
			}
			else if ((c == ASCII_XON) && ((settings.flowControlType == FCT_AUTOOFF) || (settings.flowControlType == FCT_MANUAL)))
			{
				DPRINTLN("packetXOn = true");
				if (settings.flowControlType == FCT_MANUAL)
				{
					DPRINTLN("sendNextPacket()");
				}
			}
			else
			{
				if (settings.doEcho)
				{
					serial->write(c);
				}
				if ((c == BS) || ((BS == 8) && ((c == ASCII_DC4) || (c == ASCII_DELETE))))
				{
					if (buflen > 0)
					{
						buffer[--buflen] = '\0';
					}
					continue;
				}
				buffer[buflen++] = c;
				buffer[buflen] = '\0';
				crReceived = (buflen >= MAX_COMMAND_SIZE) || (buflen == 2 && buffer[1] == '/' && lc(buffer[0]) == 'a');
			}
		}
	}
	return crReceived;
}

void ZModem::showInitMessage()
{
	serial->print(settings.EOLN);
	serial->print("ZModem Firmware v");
	serial->print(ZMODEM_VERSION);
	serial->print(settings.EOLN);

	serial->printf("sdk=%s chipid=%d cpu@%d", ESP.getSdkVersion(), ESP.getChipRevision(), ESP.getCpuFreqMHz());
	serial->print(settings.EOLN);

	serial->printf("totsize=%dk hsize=%dk fsize=%dk speed=%dm", (ESP.getFlashChipSize() / 1024), (ESP.getFreeHeap() / 1024), SPIFFS.totalBytes() / 1024, (ESP.getFlashChipSpeed() / 1000000));
	serial->print(settings.EOLN);

	if (settings.wifiSSID.length() > 0)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			serial->print(("CONNECTED TO " + settings.wifiSSID + " (" + WiFi.localIP().toString().c_str() + ")").c_str());
		}
		else
		{
			serial->print(("ERROR ON " + settings.wifiSSID).c_str());
		}
	}
	else
	{
		serial->print("INITIALIZED");
	}
	serial->print(settings.EOLN);
	serial->print("READY.");
	serial->print(settings.EOLN);
	serial->flush();
}

void ZModem::sendResponse(ZResult rc)
{
	if (rc < ZIGNORE)
	{
		DPRINTF("Response: %s\n", RESULT_CODES_V1[rc]);
		serial->print(settings.EOLN);
		serial->print(settings.numericResponses ? RESULT_CODES_V0[rc] : RESULT_CODES_V1[rc]);
		serial->print(settings.EOLN);
	}
	else
	{
		DPRINTF("Response: %d\n", rc);
	}
}

void ZModem::sendConnectionNotice(int id)
{
	serial->print(settings.EOLN);
	if (settings.numericResponses)
	{
		if (!settings.longResponses)
		{
			serial->print("1");
		}
		else if (settings.baudRate < 1200)
		{
			serial->print("1");
		}
		else if (settings.baudRate < 2400)
		{
			serial->print("5");
		}
		else if (settings.baudRate < 4800)
		{
			serial->print("10");
		}
		else if (settings.baudRate < 7200)
		{
			serial->print("11");
		}
		else if (settings.baudRate < 9600)
		{
			serial->print("24");
		}
		else if (settings.baudRate < 12000)
		{
			serial->print("12");
		}
		else if (settings.baudRate < 14400)
		{
			serial->print("25");
		}
		else if (settings.baudRate < 19200)
		{
			serial->print("13");
		}
		else
		{
			serial->print("28");
		}
	}
	else
	{
		serial->print("CONNECT");
		if (settings.longResponses)
		{
			serial->print(" ");
			serial->print(id);
		}
	}
	serial->print(settings.EOLN);
}

size_t ZModem::socketWrite(uint8_t c)
{
	size_t totalBytesSent = 0;

	if (c == 0xFF && socket->telnetMode())
	{
		totalBytesSent += socket->write(c);
	}
	totalBytesSent += socket->write(c);

	return totalBytesSent;
}

size_t ZModem::socketWrite(const uint8_t *buf, size_t size)
{
	if (socket->telnetMode())
	{
		uint8_t escbuf[size * 2];
		int k = 0;
		for (int i = 0; i < size; i++)
		{
			escbuf[k++] = buf[i];
			if (buf[i] == 0xFF)
			{
				escbuf[k++] = buf[i];
			}
		}
		return socket->write(escbuf, k);
	}
	return socket->write(buf, size);
}

uint8_t ZModem::socketRead(unsigned long tmout)
{
	if (!socket->available())
	{
		unsigned long start = millis();
		while (socket->available() == 0 && (millis() - start) < tmout)
			delay(1);
	}
	return socket->read();
}

ZResult ZModem::execCommand()
{
	String sbuf = (char *)buffer;
	int len = buflen;
	buffer[0] = '\0';
	buflen = 0;

	sbuf.trim();

	if (sbuf.length() == 2 && lc(sbuf[0]) == 'a' && lc(sbuf[1]) == '/')
	{
		sbuf = lastCommand;
		len = lastCommand.length();
	}

	int i = 0;

	while (i < len - 1 && (lc(sbuf[i]) != 'a' || lc(sbuf[i + 1]) != 't'))
	{
		i++;
	}

	if (i < len - 1 && lc(sbuf[i]) == 'a' && lc(sbuf[i + 1]) == 't')
	{
		i += 2;
		char cmd = ' ';
		char sec = ' ';
		int vstart = 0;
		int vlen = 0;
		String dmodifiers = "";
		ZResult rc = ZOK;

		while (i < len)
		{
			while (i < len && (sbuf[i] == ' ' || sbuf[i] == '\t'))
			{
				i++;
			}
			cmd = lc(sbuf[i++]);
			vstart = i;
			vlen = 0;
			bool isNumber = true;
			if (i < len)
			{
				if (cmd == '+' || cmd == '$')
				{
					vlen += len - i;
					i = len;
				}
				else if (cmd == '&' || cmd == '%')
				{
					i++;
					sec = lc(sbuf[vstart]);
					vstart++;
				}
			}
			while (i < len && (sbuf[i] == ' ' || sbuf[i] == '\t'))
			{
				vstart++;
				i++;
			}
			if (i < len)
			{
				if (sbuf[i] == '\"')
				{
					isNumber = false;
					vstart++;
					while (++i < len && (sbuf[i] != '\"' || sbuf[i - 1] == '\\'))
					{
						vlen++;
					}
					if (i < len)
					{
						i++;
					}
				}
				else if (strchr("dcpatw", cmd) != NULL)
				{
					const char *DMODIFIERS = ",exprts+";
					while (i < len && (strchr(DMODIFIERS, lc(sbuf[i])) != NULL))
					{
						dmodifiers += lc((char)sbuf[i++]);
					}
					while (i < len && (sbuf[i] == ' ' || sbuf[i] == '\t'))
					{
						i++;
					}
					vstart = i;
					if (sbuf[i] == '\"')
					{
						vstart++;
						while (++i < len && (sbuf[i] != '\"' || sbuf[i - 1] == '\\'))
						{
							vlen++;
						}
						if (i < len)
						{
							i++;
						}
					}
					else
					{
						vlen += len - i;
						i = len;
					}
					for (int k = vstart; k < vstart + vlen; k++)
					{
						char c = sbuf[k];
						isNumber = ((c == '-') || ((c >= '0') && (c <= '9'))) && isNumber;
					}
				}
				else
				{
					while ((i < len) && (!((lc(sbuf[i]) >= 'a') && (lc(sbuf[i]) <= 'z'))) && (sbuf[i] != '&') && (sbuf[i] != '%') && (sbuf[i] != ' '))
					{
						char c = sbuf[i];
						isNumber = ((c == '-') || ((c >= '0') && (c <= '9'))) && isNumber;
						vlen++;
						i++;
					}
				}
			}
			long vval = 0;
			uint8_t vbuf[vlen + 1];
			memset(vbuf, 0, vlen + 1);
			if (vlen > 0)
			{
				memcpy(vbuf, sbuf.c_str() + vstart, vlen);
				if (vlen > 0 && isNumber)
				{
					String num = "";
					for (uint8_t *v = vbuf; v < (vbuf + vlen); v++)
					{
						if ((*v >= '0') && (*v <= '9'))
						{
							num += (char)*v;
						}
					}
					vval = atol(num.c_str());
				}
			}
			DPRINTF("Command: %s\n", sbuf.c_str());
			if (vlen > 0)
			{
				DPRINTF("Proc: %c %lu '%s'\n", cmd, vval, vbuf);
			}
			else
			{
				DPRINTF("Proc: %c %lu ''\n", cmd, vval);
			}
			switch (cmd)
			{
			case 'z':
				rc = execReset();
				break;
			case 'n':
				DPRINTLN("listen");
				break;
			case 'a':
				DPRINTLN("answer");
				break;
			case 'e':
				if (!isNumber)
				{
					rc = ZERROR;
				}
				else
				{
					settings.doEcho = (vval > 0);
				}
				break;
			case 'f':
				DPRINTLN("flowcontrol");
				break;
			case 'x':
				DPRINTLN("x");
				break;
			case 'r':
				rc = execEOLN(vval, vbuf, vlen, isNumber);
				break;
			case 'b':
				rc = execBaud(vval, vbuf, vlen);
				break;
			case 't':
				DPRINTLN("t");
				break;
			case 'h':
				rc = execHangup(vval, vbuf, vlen, isNumber);
				break;
			case 'd':
				rc = execDial(vval, vbuf, vlen, isNumber, dmodifiers.c_str());
				break;
			case 'p':
				rc = execPhonebook(vval, vbuf, vlen, isNumber, dmodifiers.c_str());
				break;
			case 'o':
				if (vlen == 0 || vval == 0)
				{
					if (socket == nullptr || !socket->connected())
					{
						rc = ZERROR;
					}
					else
					{
						switchTo(ZSTREAM_MODE);
						rc = ZOK;
					}
				}
				else
				{
					rc = isNumber ? execDial(vval, vbuf, vlen, isNumber, "") : ZERROR;
				}
				break;
			case 'c':
				rc = execConnect(vval, vbuf, vlen, isNumber, dmodifiers.c_str());
				break;
			case 'i':
				rc = execInfo(vval, vbuf, vlen, isNumber);
				break;
			case 'l':
				DPRINTLN("l");
				break;
			case 'm':
				DPRINTLN("m");
				break;
			case 'y':
				rc = isNumber ? ZOK : ZERROR;
				break;
			case 'w':
				rc = execWiFi(vval, vbuf, vlen, isNumber, dmodifiers.c_str());
				break;
			case 'v':
				if (!isNumber)
				{
					rc = ZERROR;
				}
				else
				{
					settings.numericResponses = (vval == 0);
				}
				break;
			case 'q':
				if (!isNumber)
				{
					rc = ZERROR;
				}
				else
				{
					settings.suppressResponses = (vval > 0);
				}
				break;
			case 's':
				DPRINTLN("s");
				break;
			case '+':
				DPRINTLN("+");
				break;
			case '$':
			{
				int eqMark = 0;
				for (int i = 0; vbuf[i] != '\0'; i++)
				{
					if (vbuf[i] == '=')
					{
						eqMark = i;
						break;
					}
					else
					{
						vbuf[i] = lc(vbuf[i]);
					}
				}
				if (!eqMark)
				{
					rc = ZERROR;
				}
				else
				{
					vbuf[eqMark] = '\0';
					String var = (char *)vbuf;
					var.trim();
					String val = (char *)(vbuf + eqMark + 1);
					val.trim();
					rc = (val.length() == 0 && strcmp(var.c_str(), "pass") != 0) ? ZERROR : ZOK;
					if (rc == ZOK)
					{
						DPRINTLN(var);
						DPRINTLN(val);
						if (strcmp(var.c_str(), "ssid") == 0)
						{
							settings.wifiSSID = val;
						}
						else if (strcmp(var.c_str(), "pass") == 0)
						{
							settings.wifiPSWD = val;
						}
						else if (strcmp(var.c_str(), "mdns") == 0)
						{
							settings.hostname = val;
						}
						else if (strcmp(var.c_str(), "sb") == 0)
						{
							rc = execBaud(atoi(val.c_str()), (uint8_t *)val.c_str(), val.length());
						}
						else
						{
							rc = ZERROR;
						}
					}
				}
				break;
			}

			case '%':
				rc = ZERROR;
				break;
			case '&':
				switch (sec)
				{
				case 'k':
					DPRINTLN("k");
					break;
				case 'l':
					if (isNumber)
						settings.loadUserProfile(vval);
					else
						rc = ZERROR;
					break;
				case 'w':
					if (isNumber)
						settings.saveUserProfile(vval);
					else
						rc = ZERROR;
					break;
				case 'f':
					if (isNumber)
						settings.loadFactoryProfile(vval);
					else
						rc = ZERROR;
					break;
				case 'm':
					DPRINTLN("m");
					break;
				case 'y':
					DPRINTLN("y");
					break;
				case 'd':
					DPRINTLN("d");
					break;
				case 'o':
					DPRINTLN("o");
					break;
				case 'h':
					DPRINTLN("h");
					break;
				case 'g':
					DPRINTLN("g");
					break;
				case 's':
					DPRINTLN("s");
					break;
				case 'p':
					DPRINTLN("p");
					break;
				case 'n':
					DPRINTLN("n");
					break;
				case 't':
					rc = execTime(vval, vbuf, vlen, isNumber);
					break;
				case 'u':
					DPRINTLN("u");
					break;
				default:
					rc = ZERROR;
				}
				break;
			default:
				rc = ZERROR;
			}
		}
		lastCommand = sbuf;
		return rc;
	}
	return ZERROR;
}

ZResult ZModem::execReset()
{
	for (int i = 0; i < clients.size(); i++)
	{
		ZClient *c = clients.get(i);
		c->stop();
		delay(50);
		delete c;
	}
	clients.clear();
	socket = nullptr;
	settings.loadUserProfile(0);
	return ZOK;
}

ZResult ZModem::execInfo(int vval, uint8_t *vbuf, int vlen, bool isNumber)
{
	switch (vval)
	{
	case 0:
		showInitMessage();
		break;
	case 1:
	case 5:
		serial->print(settings.EOLN);
		serial->print("AT");
		serial->printf("%s%d", "B", settings.baudRate);
		serial->printf("%s%d", "E", settings.doEcho ? 1 : 0);
		serial->printf("%s%d", "Q", settings.suppressResponses ? 1 : 0);
		if (vval == 5)
		{
			serial->printf("%s%d", "V", settings.numericResponses ? 1 : 0);
			serial->printf("%s%d", "X", settings.longResponses ? 1 : 0);
		}
		switch (settings.flowControlType)
		{
		case FCT_RTSCTS:
			serial->printf("%s%d", "F", 0);
			break;
		case FCT_NORMAL:
			serial->printf("%s%d", "F", 1);
			break;
		case FCT_AUTOOFF:
			serial->printf("%s%d", "F", 2);
			break;
		case FCT_MANUAL:
			serial->printf("%s%d", "F", 3);
			break;
		case FCT_DISABLED:
			serial->printf("%s%d", "F", 4);
			break;
		case FCT_INVALID:
			break;
		}
		if (settings.EOLN == CR)
			serial->printf("%s%d", "R", 0);
		else if (settings.EOLN == CRLF)
			serial->printf("%s%d", "R", 1);
		else if (settings.EOLN == LFCR)
			serial->printf("%s%d", "R", 2);
		else if (settings.EOLN == LF)
			serial->printf("%s%d", "R", 3);
		break;
	case 2:
		serial->print(settings.EOLN);
		serial->print(WiFi.localIP().toString());
		break;
	case 3:
		serial->print(settings.EOLN);
		serial->print(settings.wifiSSID);
		break;
	case 4:
		serial->print(settings.EOLN);
		serial->print(ZMODEM_VERSION);
		break;
	case 6:
		serial->print(settings.EOLN);
		serial->print(WiFi.macAddress());
		break;
	case 8:
		serial->print(settings.EOLN);
		serial->print(compile_date);
		break;
	case 9:
		serial->print(settings.EOLN);
		serial->print(settings.wifiSSID);
		if (staticIP != nullptr)
		{
			serial->print(settings.EOLN);
			serial->print(staticIP->toString());
			serial->print(settings.EOLN);
			serial->print(staticSN->toString());
			serial->print(settings.EOLN);
			serial->print(staticGW->toString());
			serial->print(settings.EOLN);
			serial->print(staticDNS->toString());
		}
		break;
	case 11:
		serial->print(settings.EOLN);
		serial->print(ESP.getFreeHeap());
		break;
	default:
		serial->print(settings.EOLN);
		return ZERROR;
	}

	return ZOK;
}

ZResult ZModem::execTime(int vval, uint8_t *vbuf, int vlen, bool isNumber)
{
	struct tm now;
	if (getLocalTime(&now))
	{
		serial->print(settings.EOLN);
		serial->print(&now, "%A, %B %d %Y %H:%M:%S");
		return ZOK;
	}
	return ZERROR;
}

ZResult ZModem::execWiFi(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
{
	if (vlen == 0 || vval > 0)
	{
		int n = WiFi.scanNetworks();
		if (vval > 0 && vval < n)
		{
			n = vval;
		}
		serial->print(settings.EOLN);
		for (int i = 0; i < n; ++i)
		{
			serial->print(WiFi.SSID(i));
			serial->print(" (");
			serial->print(WiFi.RSSI(i));
			serial->print(")");
			serial->print(WiFi.encryptionType(i) == ENC_TYPE_NONE ? " " : "*");
			serial->print(settings.EOLN);
			delay(10);
		}
	}
	else
	{
		char *x = strstr((char *)vbuf, ",");
		char *ssid = (char *)vbuf;
		char *pswd = ssid + strlen(ssid);
		IPAddress *ip[4];
		for (int i = 0; i < 4; i++)
		{
			ip[i] = nullptr;
		}
		if (x > 0)
		{
			*x = 0;
			pswd = x + 1;
			x = strstr(pswd, ",");
			if (x > 0)
			{
				int numCommasFound = 0;
				int numDotsFound = 0;
				char *comPos[4];
				for (char *e = pswd + strlen(pswd) - 1; e > pswd; e--)
				{
					if (*e == ',')
					{
						if (numDotsFound != 3)
						{
							break;
						}
						numDotsFound = 0;
						if (numCommasFound < 4)
						{
							numCommasFound++;
							comPos[4 - numCommasFound] = e;
						}
						if (numCommasFound == 4)
						{
							break;
						}
					}
					else if (*e == '.')
					{
						numDotsFound++;
					}
					else if (strchr("0123456789 ", *e) == NULL)
					{
						break;
					}
				}
				if (numCommasFound == 4)
				{
					for (int i = 0; i < 4; i++)
					{
						*(comPos[i]) = 0;
					}
					for (int i = 0; i < 4; i++)
					{
						ip[i] = ZSettings::parseIP(comPos[i] + 1);
						if (ip[i] == NULL)
						{
							while (--i >= 0)
							{
								free(ip[i]);
								ip[i] = nullptr;
							}
							break;
						}
					}
				}
			}
		}

		if (!connectWiFi(ssid, pswd, ip[0], ip[1], ip[2], ip[3]))
		{
			for (int i = 0; i < 4; i++)
			{
				if (ip[i] != nullptr)
				{
					free(ip[i]);
				}
			}
			digitalWrite(PIN_LED_WIFI, LOW);
			return ZERROR;
		}
		settings.wifiSSID = ssid;
		settings.wifiPSWD = pswd;
		setStaticIPs(ip[0], ip[1], ip[2], ip[3]);
		digitalWrite(PIN_LED_WIFI, HIGH);
	}
	return ZOK;
}

ZResult ZModem::execEOLN(int vval, uint8_t *vbuf, int vlen, bool isNumber)
{
	if (isNumber)
	{
		if (vval >= 0 && vval < 4)
		{
			switch (vval)
			{
			case 0:
				settings.EOLN = CR;
				DPRINTF("EOLN = %s\n", "\\r");
				break;
			case 1:
				settings.EOLN = CRLF;
				DPRINTF("EOLN = %s\n", "\\r\\n");
				break;
			case 2:
				settings.EOLN = LFCR;
				DPRINTF("EOLN = %s\n", "\\n\\r");
				break;
			case 3:
				settings.EOLN = LF;
				DPRINTF("EOLN = %s\n", "\\n");
				break;
			}
			return ZOK;
		}
	}
	return ZERROR;
}

ZResult ZModem::execBaud(int vval, uint8_t *vbuf, int vlen)
{
	DPRINTF("change baud rate to: %d\n", vval);
	serial->flush();
	delay(500);
	serial->end();

	settings.baudRate = vval;
	serial->begin(vval);
	digitalWrite(PIN_LED_HS, vval >= DEFAULT_HS_RATE ? HIGH : LOW);

	return ZOK;
}

ZResult ZModem::execDial(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
{
	if (vlen == 0)
	{
		if (socket == nullptr || !socket->connected())
		{
			return ZERROR;
		}
		switchTo(ZSTREAM_MODE);
	}
	else if (vval >= 0 && isNumber)
	{
		DPRINTF("Phonebook entry #%lu\n", vval);
		int i = phonebook.indexOf(vval);
		if (i >= 0)
		{
			PBEntry pbe;
			phonebook.get(i, &pbe);
			return execDial(0, (uint8_t *)pbe.address, strlen(pbe.address), false, pbe.modifiers);
		}
		for (i = 0; i < clients.size(); i++)
		{
			ZClient *c = clients.get(i);
			if (c->id() == vval && c->connected())
			{
				socket = c;
				switchTo(ZSTREAM_MODE);
				return ZCONNECT;
			}
		}
		return ZERROR;
	}
	else
	{
		char *colon = strstr((char *)vbuf, ":");
		int port = 23;
		if (colon != NULL)
		{
			*colon = '\0';
			port = atoi((char *)(++colon));
		}
		DPRINTF("Connecting to %s:%d ", (char *)vbuf, port);
		ZClient *client = new ZClient();
		if (client->connect((char *)vbuf, port))
		{
			DPRINTLN("OK");
			client->setNoDelay(true);
			if (strchr(dmodifiers, 'p') != NULL || strchr(dmodifiers, 'P') != NULL)
				client->setPetsciiMode(true);
			if (strchr(dmodifiers, 't') != NULL || strchr(dmodifiers, 'T') != NULL)
				client->setTelnetMode(true);
			socket = client;
			clients.add(client);
			switchTo(ZSTREAM_MODE);
			return ZCONNECT;
		}
		DPRINTLN("FAILED");
		delete client;
		return ZNOANSWER;
	}
	return ZOK;
}

ZResult ZModem::execConnect(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
{
	if (vlen == 0)
	{
		// ATC : Shows information about the current network connection in the format
		// [CONNECTION STATE] [CONNECTION ID] [CONNECTED TO HOST]:[CONNECTED TO PORT]
		if (strlen(dmodifiers) > 0)
		{
			return ZERROR;
		}
		if (socket == nullptr)
		{
			return ZERROR;
		}
		if (socket->connected())
		{
			serial->print(settings.EOLN);
			serial->printf("%s %d %s:%d", "CONNECTED", socket->id(), socket->host(), socket->port());
		}
		else if (socket->answered())
		{
			serial->print(settings.EOLN);
			serial->printf("%s %d %s:%d", "NO CARRIER", socket->id(), socket->host(), socket->port());
		}
	}
	else if (isNumber)
	{
		if (strlen(dmodifiers) > 0)
		{
			return ZERROR;
		}
		// ATC0 Lists information about all of the network connections in the format
		// [CONNECTION STATE] [CONNECTION ID] [CONNECTED TO HOST]:[CONNECTED TO PORT]
		// including any Server (ATA) listeners.
		if (vval == 0)
		{
			for (int i = 0; i < clients.size(); i++)
			{
				ZClient *c = clients.get(i);
				if (c->connected())
				{
					serial->print(settings.EOLN);
					serial->printf("%s %d %s:%d", "CONNECTED", c->id(), c->host(), c->port());
				}
				else if (socket->answered())
				{
					serial->print(settings.EOLN);
					serial->printf("%s %d %s:%d", "NO CARRIER", c->id(), c->host(), c->port());
				}
			}
			return ZOK;
		}
		else
		{
			// ATCn (n > 0) changes the current connection to the one with the given ID.
			// If no connection exists with the given id, ERROR is returned.
			for (int i = 0; i < clients.size(); i++)
			{
				ZClient *c = clients.get(i);
				if (c->id() == vval)
				{
					socket = c;
					return ZOK;
				}
			}
		}
	}
	else
	{
		// ATC"[HOSTNAME]:[PORT]" creates a new connection to the given host and port,
		// assigning a new id if the connection is successful, and making this connection
		// the new current connection.  The quotes and colon are required.
		char *colon = strstr((char *)vbuf, ":");
		int port = 23;
		if (colon != NULL)
		{
			*colon = '\0';
			port = atoi((char *)(++colon));
		}
		DPRINTF("Connecting to %s:%d ", (char *)vbuf, port);
		ZClient *client = new ZClient();
		if (client->connect((char *)vbuf, port))
		{
			DPRINTLN("OK");
			client->setNoDelay(true);
			if (strchr(dmodifiers, 'p') != NULL || strchr(dmodifiers, 'P') != NULL)
				client->setPetsciiMode(true);
			if (strchr(dmodifiers, 't') != NULL || strchr(dmodifiers, 'T') != NULL)
				client->setTelnetMode(true);
			clients.add(client);
			socket = client;
			return ZCONNECT;
		}
		DPRINTLN("FAILED");
		delete client;
		return ZNOANSWER;
	}
	return ZOK;
}

ZResult ZModem::execHangup(int vval, uint8_t *vbuf, int vlen, bool isNumber)
{
	if (vlen == 0)
	{
		for (int i = 0; i < clients.size(); i++)
		{
			ZClient *c = clients.get(i);
			c->stop();
			delete c;
		}
		clients.clear();
		socket = nullptr;
		return ZOK;
	}
	if (isNumber)
	{
		if (vval == 0 && socket != nullptr)
		{
			DPRINTLN("Hangup current");
			socket->stop();
			for (int i = 0; i < clients.size(); i++)
			{
				if (clients.get(i) == socket)
				{
					DPRINTLN("found!");
					clients.remove(i);
					break;
				}
			}
			delete socket;
			socket = nullptr;
			return ZOK;
		}
		DPRINTF("Hangup: %d\n", vval);
		for (int i = 0; i < clients.size(); i++)
		{
			ZClient *c = clients.get(i);
			if (c->id() == vval)
			{
				clients.remove(i);
				c->stop();
				if (c == socket)
				{
					socket = nullptr;
				}
				delete c;
				return ZOK;
			}
		}
	}
	return ZERROR;
}

ZResult ZModem::execPhonebook(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
{
	if (vlen == 0 || isNumber || (vlen == 1 && *vbuf == '?'))
	{
		PBEntry pbe;
		for (int i = 0; i < phonebook.size(); i++)
		{
			if (phonebook.get(i, &pbe) && (!isNumber || vval == 0 || vval == pbe.number) && (strlen(dmodifiers) == 0 || modifierCompare(dmodifiers, pbe.modifiers) == 0))
			{
				serial->print(settings.EOLN);
				size_t off = serial->print(pbe.number);
				for (int i = 0; i < 10 - off; i++)
					serial->print(" ");
				serial->print(" ");
				serial->print(pbe.modifiers);
				for (int i = 1; i < 5 - strlen(pbe.modifiers); i++)
					serial->print(" ");
				serial->print(" ");
				serial->print(pbe.address);
				if (!isNumber)
				{
					serial->print(" (");
					serial->print(pbe.notes);
					serial->print(")");
				}
				delay(10);
			}
		}
		return ZOK;
	}
	char *eq = strchr((char *)vbuf, '=');
	if (eq == NULL)
		return ZERROR;
	for (char *cptr = (char *)vbuf; cptr != eq; cptr++)
		if (strchr("0123456789", *cptr) < 0)
			return ZERROR;
	char *rest = eq + 1;
	*eq = '\0';
	if (strlen((char *)vbuf) > 9)
		return ZERROR;
	unsigned long number = atol((char *)vbuf);
	int i = phonebook.indexOf(number);
	if (strcmp("delete", rest) == 0 || strcmp("DELETE", rest) == 0)
	{
		if (i < 0)
			return ZERROR;
		phonebook.remove(i);
		return ZOK;
	}
	char *colon = strchr(rest, ':');
	if (colon == NULL)
		return ZERROR;
	char *comma = strchr(colon, ',');
	char *notes = NULL;
	if (comma != NULL)
	{
		*comma = '\0';
		notes = comma + 1;
		DPRINTLN(notes);
	}
	if (!ZPhonebook::checkEntry(colon))
		return ZERROR;
	phonebook.put(number, rest, dmodifiers, notes);
	return ZOK;
}

void ZModem::switchTo(ZMode newMode, ZResult rc)
{
	switch (newMode)
	{
	case ZCOMMAND_MODE:
		DPRINTF("Switch to %s mode\n", "COMMAND");
		break;
	case ZCONFIG_MODE:
		DPRINTF("Switch to %s mode\n", "CONFIG");
		break;
	case ZSTREAM_MODE:
		DPRINTF("Switch to %s mode\n", "STREAM");
		break;
	case ZPRINT_MODE:
		DPRINTF("Switch to %s mode\n", "PRINT");
		break;
	}

	if (rc != ZIGNORE)
	{
		sendResponse(rc);
	}

	esc.gt2 = 0;
	esc.len = 0;
	mode = newMode;
}

void ZModem::commandModeHandler()
{
	if (serial->available() > 0)
	{
		bool crReceived = readSerialStream();
		if (crReceived && buflen != 0)
		{
			ZResult rc = execCommand();
			if (!settings.suppressResponses)
			{
				sendResponse(rc);
			}
		}
	}
}

void ZModem::configModeHandler()
{
}

void ZModem::streamModeHandler()
{
	if (socket != nullptr && socket->connected())
	{
		while (serial->available() > 0)
		{
			char c = serial->read();
			if (c != EC || (millis() - esc.gt1) < 1000 || esc.len >= sizeof(esc.buf))
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
		if (esc.gt2 && (millis() - esc.gt2) > 1000)
		{
			esc.gt2 = 0;
			esc.len = 0;
			switchTo(ZCOMMAND_MODE, ZOK);
		}

		if (socket->available() > 0)
		{
			int free = serial->availableForWrite();
			while (--free > 0 && socket->available() > 0)
			{
				char c = socket->read();
				if ((!socket->telnetMode() || processIAC(&c)) && (!socket->petsciiMode() || asc2pet(&c)))
					serial->write(c);
			}
		}
	}
	else
	{
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
		
		switchTo(ZCOMMAND_MODE, ZNOCARRIER);
	}
}

void ZModem::printModeHandler()
{
}

void ZModem::factoryReset()
{
	DPRINTLN("Factory Reset!");
}

void ZModem::disconnect()
{
	if (socket != nullptr)
	{
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients.get(i) == socket)
			{
				clients.remove(i);
				break;
			}
		}
		socket->flush();
		socket->stop();
		delay(500);
		delete socket;
		socket = nullptr;
	}
}

void ZModem::begin()
{
	pinMode(PIN_LED_HS, OUTPUT);
	pinMode(PIN_LED_DATA, OUTPUT);
	pinMode(PIN_LED_WIFI, OUTPUT);

	digitalWrite(PIN_LED_DATA, HIGH);
	delay(200);
	digitalWrite(PIN_LED_DATA, LOW);

	digitalWrite(PIN_LED_HS, HIGH);
	delay(200);
	digitalWrite(PIN_LED_HS, LOW);

	digitalWrite(PIN_LED_WIFI, HIGH);
	delay(200);
	digitalWrite(PIN_LED_WIFI, LOW);

	if (!SPIFFS.begin())
	{
		SPIFFS.format();
		SPIFFS.begin();
		DPRINTLN("SPIFFS Formatted.");
	}
	else
	{
		settings.loadUserProfile(0);
		phonebook.begin();
	}

	serial->begin(settings.baudRate, DEFAULT_SERIAL_CONFIG);
	serial->setRxBufferSize(MAX_COMMAND_SIZE);
	DPRINTF("COM port open at %d bit/s\n", settings.baudRate);
	digitalWrite(PIN_LED_HS, settings.baudRate >= DEFAULT_HS_RATE ? HIGH : LOW);

	if (settings.wifiSSID.length() > 0)
	{
		connectWiFi(settings.wifiSSID.c_str(), settings.wifiPSWD.c_str(), staticIP, staticDNS, staticGW, staticSN);
	}

	showInitMessage();
}

void ZModem::tick()
{
	switch (mode)
	{
	case ZCOMMAND_MODE:
		commandModeHandler();
		break;
	case ZCONFIG_MODE:
		configModeHandler();
		break;
	case ZSTREAM_MODE:
		streamModeHandler();
		break;
	case ZPRINT_MODE:
		printModeHandler();
		break;
	}
}