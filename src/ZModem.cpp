#include "ZModem.h"
#include "debug.h"
#include "CString.h"

ZModem::ZModem(HardwareSerial &serial) : serialPort(serial)
{
	echoActive = true;
	numericResponses = false;
	flowControlType = FCT_DISABLED;
	CR = '\r';
	LF = '\n';
	BS = ASCII_BS;
	EC = '+';
	strcpy(ECS, "+++");
	buffer[0] = '\0';
	buflen = 0;
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

void ZModem::showResponse(ZResult rc)
{
	switch (rc)
	{
	case ZOK:
		if (numericResponses) {
			serialPort.print("0");
		} else {
			serialPort.print("OK");
		}
		serialPort.print("\r\n");
		break;
	}
}

ZResult ZModem::doSerialCommand()
{
	DPRINTLN("doSerialCommand");

	CString cmd((char *)buffer, buflen);

	DPRINTLN(cmd);

	if (cmd.length() == 2 && lc(cmd[0]) == 'a' && lc(cmd[1]) == '/')
	{
		DPRINTLN("previous command");
	}

	int i = 0;
	ZResult rc = ZOK;

	while (i < buflen - 1 && (lc(cmd[i]) != 'a' || lc(cmd[i + 1]) != 't'))
	{
		i++;
	}

	if (i < buflen - 1 && lc(cmd[i]) == 'a' && lc(cmd[i]) == 't')
	{
	}

	return rc;
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

	serialPort.begin(DEFAULT_BAUD_RATE, DEFAULT_SERIAL_CONFIG);
	serialPort.setRxBufferSize(MAX_COMMAND_SIZE);
}

void ZModem::tick()
{
	if (serialPort.available() > 0)
	{
		bool crReceived = readSerialStream();
		clearPlusProgress();
		if (crReceived && buflen != 0)
		{
			doSerialCommand();
		}
	}
}