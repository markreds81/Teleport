#include "ZModem.h"
#include "debug.h"

ZModem::ZModem(HardwareSerial serial) : serialPort(serial)
{
	doEcho = true;
	strcpy(ECS, "+++");
}

ZModem::~ZModem()
{
}

bool ZModem::readSerialStream()
{
	bool crReceived = false;
	while (serialPort.available() > 0 && !crReceived)
	{
		uint8_t c = serialPort.read();
		if (c == '\n' || c == 'r')
		{
			if (doEcho)
			{
				serialPort.write(c);
			}
			crReceived = true;
			break;
		}

	// 	if (c > 0)
	// 	{
	// 		if (c != EC)
	// 			lastNonPlusTimeMs = millis();

	// 		if ((c == 19) && (serial.getFlowControlType() == FCT_NORMAL))
	// 		{
	// 			serial.setXON(false);
	// 		}
	// 		else if ((c == 19) && ((serial.getFlowControlType() == FCT_AUTOOFF) || (serial.getFlowControlType() == FCT_MANUAL)))
	// 		{
	// 			packetXOn = false;
	// 		}
	// 		else if ((c == 17) && (serial.getFlowControlType() == FCT_NORMAL))
	// 		{
	// 			serial.setXON(true);
	// 		}
	// 		else if ((c == 17) && ((serial.getFlowControlType() == FCT_AUTOOFF) || (serial.getFlowControlType() == FCT_MANUAL)))
	// 		{
	// 			packetXOn = true;
	// 			if (serial.getFlowControlType() == FCT_MANUAL)
	// 			{
	// 				sendNextPacket();
	// 			}
	// 		}
	// 		else
	// 		{
	// 			if (doEcho)
	// 			{
	// 				serial.write(c);
	// 				if (serial.isSerialOut())
	// 					serialOutDeque();
	// 			}
	// 			if ((c == BS) || ((BS == 8) && ((c == 20) || (c == 127))))
	// 			{
	// 				if (eon > 0)
	// 					nbuf[--eon] = 0;
	// 				continue;
	// 			}
	// 			nbuf[eon++] = c;
	// 			if ((eon >= MAX_COMMAND_SIZE) || ((eon == 2) && (nbuf[1] == '/') && lc(nbuf[0]) == 'a'))
	// 			{
	// 				eon--;
	// 				crReceived = true;
	// 			}
	// 		}
	// 	}
	}
	return crReceived;
}

void ZModem::clearPlusProgress()
{
	if (currentExpiresTimeMs > 0)
	{
		currentExpiresTimeMs = 0;
	}
	if((strcmp((char *)buffer, ECS) == 0) && ((millis() - lastNonPlusTimeMs) > 1000))
	{
		currentExpiresTimeMs = millis() + 1000;
	}
}

ZResult ZModem::doSerialCommand()
{
	DPRINTLN("doSerialCommand");
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

	serialPort.begin(DEFAULT_BAUD_RATE, DEFAULT_SERIAL_CONFIG);
	serialPort.setRxBufferSize(MAX_COMMAND_SIZE);
	
	buffer[0] = '\0';
	buflen = 0;
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