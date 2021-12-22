#include "ZModem.h"
#include "ZDebug.h"
#include "CString.h"
#include <WiFi.h>
#include <SPIFFS.h>

ZModem::ZModem(HardwareSerial &serial) : serialPort(serial)
{
	buffer[0] = '\0';
	buflen = 0;
	wifiConnected = false;
	wifiSSI = "";
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

void ZModem::setDefaults()
{
	echoActive = true;
	numericResponses = false;
	flowControlType = FCT_DISABLED;
	BS = ASCII_BS;
	EC = '+';
	strcpy(CRLF, "\r\n");
	strcpy(LFCR, "\n\r");
	strcpy(LF, "\n");
	strcpy(CR, "\r");
	memset(ECS, EC, 3);
	EOLN = CRLF;
}

bool ZModem::readSerialStream()
{
	bool crReceived = false;
	while (serialPort.available() > 0 && !crReceived)
	{
		uint8_t c = serialPort.read();
		if (c == '\n' || c == '\r')
		{
			if (echoActive)
			{
				serialPort.write(c);
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

			if ((c == ASCII_XOFF) && (flowControlType == FCT_NORMAL))
			{
				DPRINTLN("serial.setXON(false)");
			}
			else if ((c == ASCII_XOFF) && ((flowControlType == FCT_AUTOOFF) || (flowControlType == FCT_MANUAL)))
			{
				DPRINTLN("packetXOn = false");
			}
			else if ((c == ASCII_XON) && (flowControlType == FCT_NORMAL))
			{
				DPRINTLN("serial.setXON(true)");
			}
			else if ((c == ASCII_XON) && ((flowControlType == FCT_AUTOOFF) || (flowControlType == FCT_MANUAL)))
			{
				DPRINTLN("packetXOn = true");
				if (flowControlType == FCT_MANUAL)
				{
					DPRINTLN("sendNextPacket()");
				}
			}
			else
			{
				if (echoActive)
				{
					serialPort.write(c);
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
				if ((buflen >= MAX_COMMAND_SIZE) || ((buflen == 2) && (buffer[1] == '/') && lc(buffer[0]) == 'a'))
				{
					buflen--;
					crReceived = true;
				}
			}
		}
	}
	return crReceived;
}

void ZModem::showInitMessage()
{
	serialPort.print(EOLN);
	serialPort.print("ZModem Firmware v");
	serialPort.print(ZMODEM_VERSION);
	serialPort.print(EOLN);
	
	serialPort.printf("sdk=%s chipid=%d cpu@%d", ESP.getSdkVersion(), ESP.getChipRevision(), ESP.getCpuFreqMHz());
	serialPort.print(EOLN);

	serialPort.printf("totsize=%dk hsize=%dk fsize=%dk speed=%dm", (ESP.getFlashChipSize()/1024), (ESP.getFreeHeap()/1024),SPIFFS.totalBytes()/1024, (ESP.getFlashChipSpeed()/1000000));
	serialPort.print(EOLN);

	if (wifiSSI.length() > 0)
	{
		if (wifiConnected)
		{
			serialPort.print(("CONNECTED TO " + wifiSSI + " (" + WiFi.localIP().toString().c_str() + ")").c_str());
		}
		else
		{
			serialPort.print(("ERROR ON " + wifiSSI).c_str());
		}
	}
	else
	{
		serialPort.print("INITIALIZED");
	}
	serialPort.print(EOLN);
	serialPort.print("READY.");
	serialPort.print(EOLN);
	serialPort.flush();
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
	switch (rc)
	{
	case ZOK:
		DPRINTF("Response: %s\n", "OK");
		serialPort.print(EOLN);
		if (numericResponses)
		{
			serialPort.print("0");
		}
		else
		{
			serialPort.print("OK");
		}
		serialPort.print(EOLN);
		break;
	case ZERROR:
		DPRINTF("Response: %s\n", "ERROR");
		serialPort.print(EOLN);
		if (numericResponses)
		{
			serialPort.print("4");
		}
		else
		{
			serialPort.print("ERROR");
		}
		serialPort.print(EOLN);
		break;
	case ZNOANSWER:
		DPRINTF("Response: %s\n", "NOANSWER");
		serialPort.print(EOLN);
		if (numericResponses)
		{
			serialPort.print("8");
		}
		else
		{
			serialPort.print("NO ANSWER");
		}
		serialPort.print(EOLN);
		break;
	case ZCONNECT:
		DPRINTF("Response: %s\n", "connected");
		break;
	default:
		DPRINTF("Response: %d\n", rc);
	}
}

ZResult ZModem::execSerialCommand()
{
	String sbuf = (char *)buffer;
	int len = buflen;
	buffer[0] = '\0';
	buflen = 0;

	sbuf.trim();

	if (sbuf.length() == 2 && lc(sbuf[0]) == 'a' && lc(sbuf[1]) == '/')
	{
		DPRINTLN("previous command");
	}

	int i = 0;
	ZResult rc = ZOK;

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
					for (int k = vstart; k < vstart + len; k++)
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
				DPRINTLN("echo");
				break;
			case 'f':
				DPRINTLN("flowcontrol");
				break;
			case 'x':
				DPRINTLN("x");
				break;
			case 'r':
				DPRINTLN("r");
				break;
			case 'b':
				DPRINTLN("b");
				break;
			case 't':
				DPRINTLN("t");
				break;
			case 'h':
				DPRINTLN("h");
				break;
			case 'd':
				DPRINTLN("d");
				break;
			case 'p':
				DPRINTLN("p");
				break;
			case 'o':
				DPRINTLN("o");
				break;
			case 'c':
				DPRINTLN("c");
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
				DPRINTLN("y");
				break;
			case 'w':
				DPRINTLN("w");
				break;
			case 'v':
				DPRINTLN("v");
				break;
			case 'q':
				DPRINTLN("q");
				break;
			case 's':
				DPRINTLN("s");
				break;
			case '+':
				DPRINTLN("+");
				break;
			case '$':
				DPRINTLN("$");
				break;
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
					DPRINTLN("loadConfig");
					break;
				case 'w':
					DPRINTLN("saveConfig");
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

		if (rc != ZOK || i >= len)
		{
			sendResponse(rc);
		}
	}

	return rc;
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
			break;
		case 2:
			serialPort.print(WiFi.localIP().toString());
			serialPort.print(EOLN);
			break;
		case 3:
			serialPort.print(wifiSSI);
    		serialPort.print(EOLN);
			break;
		case 4:
			serialPort.print(ZMODEM_VERSION);
    		serialPort.print(EOLN);
			break;
		case 6:
			serialPort.print(WiFi.macAddress());
			serialPort.print(EOLN);
			break;
		case 7:
			break;
		case 8:
			serialPort.print(compile_date);
			serialPort.print(EOLN);
			break;
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		default:
			return ZERROR;
	}
	return ZOK;
}

void ZModem::factoryReset()
{
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

	serialPort.begin(DEFAULT_BAUD_RATE, DEFAULT_SERIAL_CONFIG);
	serialPort.setRxBufferSize(MAX_COMMAND_SIZE);

	setDefaults();
}

void ZModem::tick()
{
	if (serialPort.available() > 0)
	{
		bool crReceived = readSerialStream();
		clearPlusProgress();
		if (crReceived && buflen != 0)
		{
			execSerialCommand();
		}
	}
}