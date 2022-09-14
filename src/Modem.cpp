#include "Modem.h"
#include "Phonebook.h"
#include "version.h"
#include <SPIFFS.h>
#include <WiFi.h>

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

const char *const Modem::RESULT_CODES_V0[] = {
	"0", "1", "2", "3", "4", "6", "7", "8"};

const char *const Modem::RESULT_CODES_V1[] = {
	"OK",
	"CONNECT",
	"RING",
	"NO CARRIER",
	"ERROR",
	"NO DIALTONE",
	"BUSY",
	"NO ANSWER"};

const unsigned char Modem::PET2ASC_TABLE[256] PROGMEM = {
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

const unsigned char Modem::ASC2PET_TABLE[256] PROGMEM = {
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

Modem::Modem()
{
	mode = ZCOMMAND_MODE;
	buffer[0] = '\0';
	buflen = 0;
	termType = DEFAULT_TERMTYPE;
	lastCommand = emptyString;
	memset(&esc, 0, sizeof(esc));
}

Modem::~Modem()
{
}

char Modem::lc(char c)
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

bool Modem::asc2pet(char *c)
{
	return true;
}

bool Modem::processIAC(char *c)
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

int Modem::modifierCompare(const char *m1, const char *m2)
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

void Modem::setStaticIPs(IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet)
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

bool Modem::connectWiFi(const char *ssid, const char *pswd, IPAddress *ip, IPAddress *dns, IPAddress *gateway, IPAddress *subnet)
{
	if (WiFi.status() == WL_CONNECTED)
	{
		MDNS.end();
		httpServer.stop();
		while (WiFi.status() == WL_CONNECTED)
		{
			WiFi.disconnect();
			delay(100);
		}
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
			httpServer.begin(80);
			if (strlen(SREG.hostname) > 0)
			{
				WiFi.hostname(String(SREG.hostname));
				if (MDNS.begin(SREG.hostname))
				{
					DPRINTLN("mDNS responder started");
					MDNS.addService("http", "tcp", 80);
					DPRINTF("HTTPUpdateServer available at http://%s.local/update in your browser\n", SREG.hostname);
				}
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

bool Modem::readSerialStream()
{
	bool crReceived = false;
	while (Serial2.available() > 0 && !crReceived)
	{
		uint8_t c = Serial2.read();
		if (c == '\n' || c == '\r')
		{
			if (SREG.echoEnabled())
			{
				Serial2.write(c);
			}
			crReceived = true;
			break;
		}

		if (c > 0)
		{
			if (SREG.echoEnabled())
			{
				Serial2.write(c);
			}
			if ((c == SREG[5]) || ((SREG[5] == 8) && ((c == ASCII_DC4) || (c == ASCII_DELETE))))
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
	return crReceived && buflen > 0;
}

void Modem::sendNewline()
{
	Serial2.print(SREG.carriageReturn());
	if (SREG.resultCodeVerbose())
	{
		Serial2.print(SREG.lineFeed());
	}
}

void Modem::sendAnnouncement()
{
	sendNewline();
	Serial2.printf("%s Firmware v%s (%s)", ZMODEM_APPNAME, ZMODEM_VERSION, ZMODEM_CODENAME);
	sendNewline();
	Serial2.printf("sdk=%s chipid=%d cpu@%d", ESP.getSdkVersion(), ESP.getChipRevision(), ESP.getCpuFreqMHz());
	sendNewline();
	Serial2.printf("flash=%dk heap=%dk spiffs=%dk speed=%dm", (ESP.getFlashChipSize() / 1024), (ESP.getFreeHeap() / 1024), SPIFFS.totalBytes() / 1024, (ESP.getFlashChipSpeed() / 1000000));
	sendNewline();

	if (strlen(SREG.wifiSSID) > 0)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			Serial2.printf("CONNECTED TO %s (%s)", SREG.wifiSSID, WiFi.localIP().toString().c_str());
		}
		else
		{
			Serial2.printf("ERROR ON %s", SREG.wifiSSID);
		}
	}
	else
	{
		Serial2.print("INITIALIZED");
	}
	sendNewline();
	Serial2.print("READY.");
	sendNewline();
	Serial2.flush();
}

void Modem::sendConfiguration()
{
	Profile sp;

	sendNewline();
	Serial2.printf("ACTIVE PROFILE: %d %s", SREG.baudRate, SREG.wifiSSID);
	sendNewline();
	Serial2.printf("%s%d", "B", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "E", SREG.echoEnabled() ? 1 : 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "L", SREG.speakerVolume());
	Serial2.print(' ');
	Serial2.printf("%s%d", "M", SREG.speakerControl());
	Serial2.print(' ');
	Serial2.printf("%s%d", "N", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "Q", SREG.resultCodeEnabled() ? 1 : 0);
	Serial2.print(' ');
	Serial2.print('T');
	Serial2.print(' ');
	Serial2.printf("%s%d", "V", SREG.resultCodeNumeric() ? 1 : 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "W", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "X", SREG.resultCodeExtended() ? 1 : 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "Y", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&C", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&D", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&G", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&J", 0);
	Serial2.print(' ');
	switch (SREG.flowControlMode())
	{
	case FCM_DISABLED:
		Serial2.printf("%s%d", "&K", 0);
		break;
	case FCM_UNUSED1:
		Serial2.printf("%s%d", "&K", 1);
		break;
	case FCM_UNUSED2:
		Serial2.printf("%s%d", "&K", 2);
		break;
	case FCM_HARDWARE:
		Serial2.printf("%s%d", "&K", 3);
		break;
	case FCM_SOFTWARE:
		Serial2.printf("%s%d", "&K", 4);
		break;
	case FCM_TRANSPARENT:
		Serial2.printf("%s%d", "&K", 5);
		break;
	case FCM_BOTH:
		Serial2.printf("%s%d", "&K", 6);
		break;
	case FCM_INVALID:
		break;
	}
	Serial2.print(' ');
	Serial2.printf("%s%d", "&Q", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&R", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&S", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&T", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&X", 0);
	Serial2.print(' ');
	Serial2.printf("%s%d", "&Y", 0);
	sendNewline();
	Serial2.printf("S%02d:%03d", 0, SREG[0]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 1, SREG[1]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 2, SREG[2]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 3, SREG[3]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 4, SREG[4]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 5, SREG[5]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 6, SREG[6]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 7, SREG[7]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 8, SREG[8]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 9, SREG[9]);
	Serial2.print(' ');
	sendNewline();
	Serial2.printf("S%02d:%03d", 10, SREG[10]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 11, SREG[11]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 12, SREG[12]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 18, SREG[18]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 25, SREG[25]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 26, SREG[26]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 36, SREG[36]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 37, SREG[37]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 38, SREG[38]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 46, SREG[46]);
	sendNewline();
	Serial2.printf("S%02d:%03d", 48, SREG[48]);
	Serial2.print(' ');
	Serial2.printf("S%02d:%03d", 95, SREG[95]);
	sendNewline();

	for (int num = 0; num < MAX_USER_PROFILES; num++)
	{
		sp.loadProfile(num);
		sendNewline();
		Serial2.printf("STORED PROFILE %d: %d %s", num, sp.baudRate, sp.wifiSSID);
		sendNewline();
		Serial2.printf("%s%d", "B", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "E", sp.echoEnabled() ? 1 : 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "L", sp.speakerVolume());
		Serial2.print(' ');
		Serial2.printf("%s%d", "M", sp.speakerControl());
		Serial2.print(' ');
		Serial2.printf("%s%d", "N", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "Q", sp.resultCodeEnabled() ? 1 : 0);
		Serial2.print(' ');
		Serial2.print('T');
		Serial2.print(' ');
		Serial2.printf("%s%d", "V", sp.resultCodeNumeric() ? 1 : 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "W", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "X", sp.resultCodeExtended() ? 1 : 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "Y", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&C", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&D", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&G", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&J", 0);
		Serial2.print(' ');
		switch (sp.flowControlMode())
		{
		case FCM_DISABLED:
			Serial2.printf("%s%d", "&K", 0);
			break;
		case FCM_UNUSED1:
			Serial2.printf("%s%d", "&K", 1);
			break;
		case FCM_UNUSED2:
			Serial2.printf("%s%d", "&K", 2);
			break;
		case FCM_HARDWARE:
			Serial2.printf("%s%d", "&K", 3);
			break;
		case FCM_SOFTWARE:
			Serial2.printf("%s%d", "&K", 4);
			break;
		case FCM_TRANSPARENT:
			Serial2.printf("%s%d", "&K", 5);
			break;
		case FCM_BOTH:
			Serial2.printf("%s%d", "&K", 6);
			break;
		case FCM_INVALID:
			break;
		}
		Serial2.print(' ');
		Serial2.printf("%s%d", "&Q", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&R", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&S", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&T", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&X", 0);
		Serial2.print(' ');
		Serial2.printf("%s%d", "&Y", 0);
		sendNewline();
		Serial2.printf("S%02d:%03d", 0, SREG[0]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 2, SREG[2]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 6, SREG[6]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 7, SREG[7]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 8, SREG[8]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 9, SREG[9]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 10, SREG[10]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 11, SREG[11]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 12, SREG[12]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 18, SREG[18]);
		sendNewline();
		Serial2.printf("S%02d:%03d", 36, SREG[36]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 37, SREG[37]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 40, SREG[40]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 41, SREG[41]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 46, SREG[46]);
		Serial2.print(' ');
		Serial2.printf("S%02d:%03d", 95, SREG[95]);
		sendNewline();
	}
}

void Modem::sendResponse(ZResult rc)
{
	if (rc < ZIGNORE)
	{
		DPRINTF("Response: %s\n", RESULT_CODES_V1[rc]);
		sendNewline();
		Serial2.print(SREG.resultCodeNumeric() ? RESULT_CODES_V0[rc] : RESULT_CODES_V1[rc]);
		sendNewline();
	}
	else
	{
		DPRINTF("Response: %d\n", rc);
	}
}

void Modem::sendConnectionNotice(int id)
{
	sendNewline();
	if (SREG.resultCodeNumeric())
	{
		if (!SREG.resultCodeExtended())
		{
			Serial2.print("1");
		}
		else if (SREG.baudRate < 1200)
		{
			Serial2.print("1");
		}
		else if (SREG.baudRate < 2400)
		{
			Serial2.print("5");
		}
		else if (SREG.baudRate < 4800)
		{
			Serial2.print("10");
		}
		else if (SREG.baudRate < 7200)
		{
			Serial2.print("11");
		}
		else if (SREG.baudRate < 9600)
		{
			Serial2.print("24");
		}
		else if (SREG.baudRate < 12000)
		{
			Serial2.print("12");
		}
		else if (SREG.baudRate < 14400)
		{
			Serial2.print("25");
		}
		else if (SREG.baudRate < 19200)
		{
			Serial2.print("13");
		}
		else
		{
			Serial2.print("28");
		}
	}
	else
	{
		Serial2.print("CONNECT");
		if (SREG.resultCodeExtended())
		{
			Serial2.print(" ");
			Serial2.print(id);
		}
	}
	sendNewline();
}

int Modem::activeProfile()
{
	int result = -1;

	File file = SPIFFS.open("/profile/active", "r");
	if (file)
	{
		String line = file.readString();
		if (!line.isEmpty())
		{
			result = line.toInt();
			DPRINTF("Active profile: %d\n", result);
		}			
		file.close();
	}

	return result;
}

void Modem::setActiveProfile(int num)
{
	File file = SPIFFS.open("/profile/active", "w");
	if (file)
	{
		file.print(num);
		file.close();
		DPRINTF("Set active profile: %d\n", num);
	}
}

size_t Modem::socketWrite(uint8_t c)
{
	size_t totalBytesSent = 0;

	if (c == 0xFF && socket->telnetMode())
	{
		totalBytesSent += socket->write(c);
	}
	totalBytesSent += socket->write(c);

	return totalBytesSent;
}

size_t Modem::socketWrite(const uint8_t *buf, size_t size)
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

uint8_t Modem::socketRead(unsigned long tmout)
{
	if (!socket->available())
	{
		unsigned long start = millis();
		while (socket->available() == 0 && (millis() - start) < tmout)
			delay(1);
	}
	return socket->read();
}

ZResult Modem::execCommand()
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
				if (!isNumber || vval < 0 || vval > 2)
				{
					rc = ZERROR;
				}
				else
				{
					DPRINTLN("Reset and Restore Profile");
					for (int i = 0; i < clients.size(); i++)
					{
						SocketClient *c = clients.get(i);
						c->stop();
						delay(50);
						delete c;
					}
					clients.clear();
					socket = nullptr;
					SREG.loadProfile(int(vval));
				}
				break;
			case 'n':
				DPRINTLN("listen");
				break;
			case 'a':
				DPRINTLN("answer");
				break;
			case 'e':
				if (!isNumber)
					rc = ZERROR;
				else
					SREG.setEchoEnabled(vval > 0);
				break;
			case 'f':
				DPRINTLN("f");
				break;
			case 'x':
				DPRINTLN("x");
				break;
			case 'r':
				DPRINTLN("r");
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
				if (isNumber && vval >= 0 && vval <= 3)
					SREG.setSpeakerVolume(vval);
				else
					rc = ZERROR;
				break;
			case 'm':
				if (isNumber && vval >= 0 && vval <= 3)
					SREG.setSpeakerControl(vval);
				else
					rc = ZERROR;
				break;
			case 'y':
				rc = isNumber ? ZOK : ZERROR;
				break;
			case 'w':
				rc = execWiFi(vval, vbuf, vlen, isNumber, dmodifiers.c_str());
				break;
			case 'v':
				if (!isNumber)
					rc = ZERROR;
				else
					SREG.setResultCodeNumeric(vval == 0);
				break;
			case 'q':
				if (!isNumber)
					rc = ZERROR;
				else
					SREG.setResultCodeEnabled(vval == 0);
				break;
			case 's':
				rc = execSRegister(vbuf, vlen);
				break;
			case '+':
				for (int i = 0; vbuf[i] != 0; i++)
					vbuf[i] = lc(vbuf[i]);
				if (strcmp((const char *)vbuf, "console") == 0)
					switchTo(ZCONSOLE_MODE);
				else if (strcmp((const char *)vbuf, "shell") == 0)
					switchTo(ZSHELL_MODE);
				else if (strcmp((const char *)vbuf, "rst") == 0)
					ESP.restart();
				else
					rc = ZERROR;
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
							strcpy(SREG.wifiSSID, var.c_str());
						}
						else if (strcmp(var.c_str(), "pass") == 0)
						{
							strcpy(SREG.wifiPSWD, val.c_str());
						}
						else if (strcmp(var.c_str(), "mdns") == 0)
						{
							strcpy(SREG.hostname, val.c_str());
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
					if (!isNumber || vval >= FCM_INVALID)
						rc = ZERROR;
					else
					{
						SREG.setFlowControlMode((FlowControlMode)vval);
						Serial2.setFlowControl(SREG.flowControlMode());
					}
					break;
				case 'l':
					DPRINTLN("l");
					break;
				case 'v':
					sendConfiguration();
					break;
				case 'w':
					if (isNumber && vval >= 0 && vval < MAX_USER_PROFILES)
						SREG.saveProfile(int(vval));
					else
						rc = ZERROR;
					break;
				case 'f':
					if (isNumber && vval >= 0 && vval < MAX_USER_PROFILES)
						SREG.loadProfile(-1);
					else
						rc = ZERROR;
					break;
				case 'y':
					if (isNumber && vval >= 0 && vval < MAX_USER_PROFILES)
						setActiveProfile(vval);
					else
						rc = ZERROR;
					break;
				case 'm':
					DPRINTLN("m");
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

ZResult Modem::execInfo(int vval, uint8_t *vbuf, int vlen, bool isNumber)
{
	switch (vval)
	{
	case 0:
		sendAnnouncement();
		break;
	case 1:
	case 5:
		sendConfiguration();
		break;
	case 2:
		sendNewline();
		Serial2.print(WiFi.localIP().toString());
		break;
	case 3:
		sendNewline();
		Serial2.print(SREG.wifiSSID);
		break;
	case 4:
		sendNewline();
		Serial2.print(ZMODEM_VERSION);
		break;
	case 6:
		sendNewline();
		Serial2.print(WiFi.macAddress());
		break;
	case 7:
		struct tm now;
		if (getLocalTime(&now))
		{
			sendNewline();
			Serial2.print(&now, "%A, %B %d %Y %H:%M:%S");
		}
		break;
	case 8:
		sendNewline();
		Serial2.print(compile_date);
		break;
	case 9:
		sendNewline();
		Serial2.print(SREG.wifiSSID);
		if (staticIP != nullptr)
		{
			sendNewline();
			Serial2.print(staticIP->toString());
			sendNewline();
			Serial2.print(staticSN->toString());
			sendNewline();
			Serial2.print(staticGW->toString());
			sendNewline();
			Serial2.print(staticDNS->toString());
		}
		break;
	case 11:
		sendNewline();
		Serial2.print(ESP.getFreeHeap());
		break;
	case 12:
		sendNewline();
		Serial2.printf("TX Total bytes: %lu", totalBytesTx);
		sendNewline();
		Serial2.printf("RX Total bytes: %lu", totalBytesRx);
		sendNewline();
		Serial2.printf("TX Max Rate: %lu bytes/sec", maxRateTx);
		sendNewline();
		Serial2.printf("RX Max Rate: %lu bytes/sec", maxRateRx);
		break;
	default:
		sendNewline();
		return ZERROR;
	}

	return ZOK;
}

ZResult Modem::execTime(int vval, uint8_t *vbuf, int vlen, bool isNumber)
{
	struct tm now;
	if (getLocalTime(&now))
	{
		sendNewline();
		Serial2.print(&now, "%A, %B %d %Y %H:%M:%S");
		return ZOK;
	}
	return ZERROR;
}

ZResult Modem::execWiFi(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
{
	if (vlen == 0 || vval > 0)
	{
		int n = WiFi.scanNetworks();
		if (vval > 0 && vval < n)
		{
			n = vval;
		}
		sendNewline();
		for (int i = 0; i < n; ++i)
		{
			Serial2.print(WiFi.SSID(i));
			Serial2.print(" (");
			Serial2.print(WiFi.RSSI(i));
			Serial2.print(")");
			Serial2.print(WiFi.encryptionType(i) == ENC_TYPE_NONE ? " " : "*");
			sendNewline();
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
						ip[i] = parseIP(comPos[i] + 1);
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
		strcpy(SREG.wifiSSID, ssid);
		strcpy(SREG.wifiPSWD, pswd);
		setStaticIPs(ip[0], ip[1], ip[2], ip[3]);
		digitalWrite(PIN_LED_WIFI, HIGH);
	}
	return ZOK;
}

ZResult Modem::execBaud(int vval, uint8_t *vbuf, int vlen)
{
	DPRINTF("change baud rate to: %d\n", vval);
	Serial2.flush();
	delay(500);
	Serial2.end();

	SREG.baudRate = vval;
	Serial2.begin(vval);
	digitalWrite(PIN_LED_HS, vval >= DEFAULT_HS_RATE ? HIGH : LOW);

	return ZOK;
}

ZResult Modem::execDial(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
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
		int i = Phonebook.indexOf(vval);
		if (i >= 0)
		{
			PBEntry pbe;
			Phonebook.get(i, &pbe);
			return execDial(0, (uint8_t *)pbe.address, strlen(pbe.address), false, pbe.modifiers);
		}
		for (i = 0; i < clients.size(); i++)
		{
			SocketClient *c = clients.get(i);
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
		SocketClient *client = new SocketClient();
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

ZResult Modem::execConnect(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
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
			sendNewline();
			Serial2.printf("%s %d %s:%d", "CONNECTED", socket->id(), socket->host(), socket->port());
		}
		else if (socket->answered())
		{
			sendNewline();
			Serial2.printf("%s %d %s:%d", "NO CARRIER", socket->id(), socket->host(), socket->port());
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
				SocketClient *c = clients.get(i);
				if (c->connected())
				{
					sendNewline();
					Serial2.printf("%s %d %s:%d", "CONNECTED", c->id(), c->host(), c->port());
				}
				else if (socket->answered())
				{
					sendNewline();
					Serial2.printf("%s %d %s:%d", "NO CARRIER", c->id(), c->host(), c->port());
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
				SocketClient *c = clients.get(i);
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
		SocketClient *client = new SocketClient();
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

ZResult Modem::execHangup(int vval, uint8_t *vbuf, int vlen, bool isNumber)
{
	if (vlen == 0)
	{
		for (int i = 0; i < clients.size(); i++)
		{
			SocketClient *c = clients.get(i);
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
			SocketClient *c = clients.get(i);
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

ZResult Modem::execPhonebook(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers)
{
	if (vlen == 0 || isNumber || (vlen == 1 && *vbuf == '?'))
	{
		PBEntry pbe;
		for (int i = 0; i < Phonebook.size(); i++)
		{
			if (Phonebook.get(i, &pbe) && (!isNumber || vval == 0 || vval == pbe.number) && (strlen(dmodifiers) == 0 || modifierCompare(dmodifiers, pbe.modifiers) == 0))
			{
				sendNewline();
				size_t off = Serial2.print(pbe.number);
				for (int i = 0; i < 10 - off; i++)
					Serial2.print(" ");
				Serial2.print(" ");
				Serial2.print(pbe.modifiers);
				for (int i = 1; i < 5 - strlen(pbe.modifiers); i++)
					Serial2.print(" ");
				Serial2.print(" ");
				Serial2.print(pbe.address);
				if (!isNumber)
				{
					Serial2.print(" (");
					Serial2.print(pbe.notes);
					Serial2.print(")");
				}
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
	int i = Phonebook.indexOf(number);
	if (strcmp("delete", rest) == 0 || strcmp("DELETE", rest) == 0)
	{
		if (i < 0)
			return ZERROR;
		Phonebook.remove(i);
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
	if (!Phonebook.checkEntry(colon))
		return ZERROR;
	Phonebook.put(number, rest, dmodifiers, notes);
	return ZOK;
}

ZResult Modem::execSRegister(uint8_t *vbuf, int vlen)
{
	if (vlen >= 2)
	{
		char *cmd = strchr((char *)vbuf, '?');
		if ((cmd != NULL) && (cmd != (char *)vbuf) && (cmd == (char *)&(vbuf[vlen - 1])))
		{
			*cmd = '\0';
			int snum = atoi((char *)vbuf);
			if (snum >= 0 && snum < SREG.registerSize())
			{
				sendNewline();
				Serial2.print((int)SREG[snum]);
				return ZOK;
			}
		}
		cmd = strchr((char *)vbuf, '=');
		if ((cmd != NULL) && (cmd != (char *)vbuf) && (cmd < (char *)&(vbuf[vlen - 1])))
		{
			*cmd = '\0';
			int snum = atoi((char *)vbuf);
			int sval = atoi((char *)(cmd + 1));
			if (snum >= 0 && snum < SREG.registerSize() && sval >= 0 && sval <= 255)
			{
				SREG[snum] = (uint8_t)sval;
				return ZOK;
			}
		}
	}
	return ZERROR;
}

void Modem::switchTo(ZMode newMode, ZResult rc)
{
	switch (mode)
	{
	case ZCOMMAND_MODE:
		break;
	case ZCONSOLE_MODE:
		console.end();
		break;
	case ZSTREAM_MODE:
		break;
	case ZPRINT_MODE:
		break;
	case ZSHELL_MODE:
		shell.end();
		break;
	}

	switch (newMode)
	{
	case ZCOMMAND_MODE:
		DPRINTF("Switch to %s mode\n", "COMMAND");
		break;
	case ZCONSOLE_MODE:
		DPRINTF("Switch to %s mode\n", "CONSOLE");
		console.begin(SREG);
		break;
	case ZSTREAM_MODE:
		DPRINTF("Switch to %s mode\n", "STREAM");
		break;
	case ZPRINT_MODE:
		DPRINTF("Switch to %s mode\n", "PRINT");
		break;
	case ZSHELL_MODE:
		DPRINTF("Switch to %s mode\n", "SHELL");
		shell.begin(SREG);
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

IPAddress *Modem::parseIP(const char *str)
{
    uint8_t dots[4];
    int dotDex = 0;
    char *le = (char *)str;
    const char *ld = str + strlen(str);
    
    if (strlen(str) < 7)
    {
        return NULL;
    }
        
    for (char *e = le; e <= ld; e++)
    {
        if ((*e == '.') || (e == ld))
        {
            if (le == e)
            {
                break;
            }                
            *e = 0;
            String sdot = le;
            sdot.trim();
            if ((sdot.length() == 0) || (dotDex > 3))
            {
                dotDex = 99;
                break;
            }
            dots[dotDex++] = (uint8_t)atoi(sdot.c_str());
            if (e == ld)
                le = e;
            else
                le = e + 1;
        }
    }
    if (dotDex != 4 || *le != 0)
    {
        return nullptr;
    }
        
    return new IPAddress(dots[0], dots[1], dots[2], dots[3]);
}

void Modem::factoryReset()
{
	DPRINTLN("Factory Reset!");
}

void Modem::disconnect()
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

void Modem::begin()
{
	pinMode(PIN_CTS, INPUT);
	pinMode(PIN_RTS, OUTPUT);
	pinMode(PIN_LED_HS, OUTPUT);
	pinMode(PIN_LED_DATA, OUTPUT);
	pinMode(PIN_LED_WIFI, OUTPUT);

	buzzer.playTuneAsync();

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
		SREG.loadProfile(-1);
		DPRINTLN("SPIFFS Formatted.");
	}
	else
	{
		SREG.loadProfile(activeProfile());
		Phonebook.begin();
	}

	Serial2.begin(SREG.baudRate, DEFAULT_SERIAL_CONFIG);
	Serial2.setRxBufferSize(MAX_COMMAND_SIZE);
	Serial2.setFlowControl(SREG.flowControlMode());
	DPRINTF("COM port open at %d bit/s\n", SREG.baudRate);
	digitalWrite(PIN_LED_HS, SREG.baudRate >= DEFAULT_HS_RATE ? HIGH : LOW);

	httpUpdater.setup(&httpServer);

	if (strlen(SREG.wifiSSID) > 0)
	{
		connectWiFi(SREG.wifiSSID, SREG.wifiPSWD, staticIP, staticDNS, staticGW, staticSN);
	}

	sendAnnouncement();
}
