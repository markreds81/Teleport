#include "ZModem.h"
#include "ZDebug.h"
#include "ZClient.h"
#include "ZSettings.h"
#include <WiFi.h>
#include <SPIFFS.h>

ZModem::ZModem(ZSerial *s) : serial(s), streamMode(this)
{
	mode = nullptr;
	buffer[0] = '\0';
	buflen = 0;
	BS = ASCII_BS;
	EC = '+';
	lastCommand = "";
	strcpy(CRLF, "\r\n");
	strcpy(LFCR, "\n\r");
	strcpy(LF, "\n");
	strcpy(CR, "\r");
	memset(ECS, EC, 3);
}

ZModem::~ZModem()
{
}

void ZModem::switchTo(ZMode *newMode)
{
	mode = newMode;
#if DEBUG
	if (newMode == &streamMode)
	{
		DPRINTF("Switching to %s mode\n", "stream");
	}
#endif
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
			DPRINT(".");
			delay(500);
		}
		attemps++;
	}
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
			if (c != EC)
			{
				lastNonPlusTimeMs = millis();
			}

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

void ZModem::clearPlusProgress()
{
	if (currentExpiresTimeMs > 0)
	{
		currentExpiresTimeMs = 0;
	}
	if ((strcmp((char *)buffer, ECS) == 0) && ((millis() - lastNonPlusTimeMs) > 1000))
	{
		currentExpiresTimeMs = millis() + 1000;
	}
}

void ZModem::sendResponse(ZResult rc)
{
	serial->print(settings.EOLN);
	switch (rc)
	{
	case ZOK:
		DPRINTF("Response: %s\n", "OK");
		if (settings.numericResponses)
		{
			serial->print("0");
		}
		else
		{
			serial->print("OK");
		}
		break;
	case ZERROR:
		DPRINTF("Response: %s\n", "ERROR");
		if (settings.numericResponses)
		{
			serial->print("4");
		}
		else
		{
			serial->print("ERROR");
		}
		break;
	case ZNOANSWER:
		DPRINTF("Response: %s\n", "NOANSWER");
		if (settings.numericResponses)
		{
			serial->print("8");
		}
		else
		{
			serial->print("NO ANSWER");
		}
		break;
	case ZCONNECT:
		DPRINTF("Response: %s\n", "connected");
		sendConnectionNotice(999);
		break;
	default:
		DPRINTF("Response: %d\n", rc);
	}
	serial->print(settings.EOLN);
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
					sec = sbuf[vstart];
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
				DPRINTLN("h");
				break;
			case 'd':
				rc = execDial(vval, vbuf, vlen, isNumber, dmodifiers.c_str());
				break;
			case 'p':
				DPRINTLN("p");
				break;
			case 'o':
				DPRINTLN("o");
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
					settings.load();
					break;
				case 'w':
					settings.save();
					break;
				case 'f':
					DPRINTLN("f");
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
					DPRINTLN("t");
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
	return ZERROR;
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
	default:
		serial->print(settings.EOLN);
		return ZERROR;
	}

	return ZOK;
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
		switchTo(&streamMode);			
	}
	else if (vval >= 0 && isNumber)
	{
		DPRINTF("Phonebook entry #%lu\n", vval);
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
			socket = client;
			switchTo(&streamMode);
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
	return ZOK;
}

void ZModem::switchBackToCommandMode()
{
	DPRINTLN("Switching back to command mode");
	mode = nullptr;
}

void ZModem::factoryReset()
{
}

void ZModem::disconnect()
{
	if (socket != nullptr)
	{
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
		settings.load();
	}

	serial->begin(settings.baudRate, DEFAULT_SERIAL_CONFIG);
	serial->setRxBufferSize(MAX_COMMAND_SIZE);
	DPRINTF("COM port open at %d bit/s\n", settings.baudRate);

	if (settings.wifiSSID.length() > 0)
	{
		connectWiFi(settings.wifiSSID.c_str(), settings.wifiPSWD.c_str(), staticIP, staticDNS, staticGW, staticSN);
	}

	showInitMessage();
}

void ZModem::tick()
{
	if (mode != nullptr)
	{
		mode->tick();
	}
	else if (serial->available() > 0)
	{
		bool crReceived = readSerialStream();
		clearPlusProgress();
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