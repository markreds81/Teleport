#include "StreamHandler.h"
#include "CommandHandler.h"
#include "Logger.h"
#include "AsciiUtils.h"
#include "WiFiServerNode.h"

void StreamHandler::switchTo(WiFiClientNode *conn)
{
	current = conn;
	currentExpiresTimeMs = 0;
	lastNonPlusTimeMs = 0;
	plussesInARow = 0;
	serial.setXON(true);
	serial.setPetsciiMode(isPETSCII());
	serial.setFlowControlType(getFlowControl());
	currentMode = &StreamMode;
	checkBaudChange();
	lastDTR = digitalRead(PIN_DTR);		
}

bool StreamHandler::isPETSCII()
{
	return (current != nullptr) && (current->isPETSCII());
}

bool StreamHandler::isEcho()
{
	return (current != nullptr) && (current->isEcho());
}

FlowControlType StreamHandler::getFlowControl()
{
	return (current != nullptr) ? (current->getFlowControl()) : FCT_DISABLED;
}

bool StreamHandler::isTelnet()
{
	return (current != nullptr) && (current->isTelnet());
}

bool StreamHandler::isDisconnectedOnStreamExit()
{
	return (current != nullptr) && (current->isDisconnectedOnStreamExit());
}

void StreamHandler::baudDelay()
{
	if (baudRate < 1200)
		delay(5);
	else if (baudRate == 1200)
		delay(3);
	else
		delay(1);
	yield();
}

void StreamHandler::serialIncoming()
{
	int bytesAvailable = SerialDTE.available();
	if (bytesAvailable == 0)
		return;
	uint8_t escBufDex = 0;
	while (--bytesAvailable >= 0)
	{
		uint8_t c = SerialDTE.read();
		if (((c == 27) || (escBufDex > 0)) && (!isPETSCII()))
		{
			escBuf[escBufDex++] = c;
			if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (escBufDex >= ZSTREAM_ESC_BUF_MAX) || ((escBufDex == 2) && (c != '[')))
			{
				logSerialIn(c);
				break;
			}
			if (bytesAvailable == 0)
			{
				baudDelay();
				bytesAvailable = SerialDTE.available();
			}
		}
		logSerialIn(c);
		if ((c == CommandMode.EC) && ((plussesInARow > 0) || ((millis() - lastNonPlusTimeMs) > 800)))
			plussesInARow++;
		else if (c != CommandMode.EC)
		{
			plussesInARow = 0;
			lastNonPlusTimeMs = millis();
		}
		if ((c == 19) && (getFlowControl() == FCT_NORMAL))
			serial.setXON(false);
		else if ((c == 17) && (getFlowControl() == FCT_NORMAL))
			serial.setXON(true);
		else
		{
			if (isEcho())
				serial.printb(c);
			if (isPETSCII())
				c = petToAsc(c);
			if (escBufDex == 0)
				socketWrite(c);
		}
	}

	if (escBufDex > 0)
		socketWrite(escBuf, escBufDex);
	currentExpiresTimeMs = 0;
	if (plussesInARow == 3)
		currentExpiresTimeMs = millis() + 800;
}

void StreamHandler::switchBackToCommandMode(bool logout)
{
	if (logout && (current != nullptr) && isDisconnectedOnStreamExit())
	{
		if (!CommandMode.suppressResponses)
		{
			if (CommandMode.numericResponses)
			{
				preEOLN(CommandMode.EOLN);
				serial.prints("3");
				serial.prints(CommandMode.EOLN);
			}
			else if (current->isAnswered())
			{
				preEOLN(CommandMode.EOLN);
				serial.prints("NO CARRIER");
				serial.prints(CommandMode.EOLN);
			}
		}
		delete current;
	}
	current = nullptr;
	currentMode = &CommandMode;
}

void StreamHandler::socketWrite(uint8_t *buf, uint8_t len)
{
	if (current->isConnected())
	{
		uint8_t escapedBuf[len * 2];
		if (isTelnet())
		{
			int eDex = 0;
			for (int i = 0; i < len; i++)
			{
				escapedBuf[eDex++] = buf[i];
				if (buf[i] == 0xff)
					escapedBuf[eDex++] = buf[i];
			}
			buf = escapedBuf;
			len = eDex;
		}
		for (int i = 0; i < len; i++)
			logSocketOut(buf[i]);
		current->write(buf, len);
		nextFlushMs = millis() + 250;
	}
}

void StreamHandler::socketWrite(uint8_t c)
{
	if (current->isConnected())
	{
		if (c == 0xFF && isTelnet())
			current->write(c);
		current->write(c);
		logSocketOut(c);
		nextFlushMs = millis() + 250;
		// current->flush(); // rendered safe by available check
		// delay(0);
		// yield();
	}
}

void StreamHandler::loop()
{
	WiFiServerNode *serv = servs;
	while (serv != nullptr)
	{
		if (serv->hasClient())
		{
			WiFiClient newClient = serv->server->available();
			if (newClient.connected())
			{
				int port = newClient.localPort();
				String remoteIPStr = newClient.remoteIP().toString();
				const char *remoteIP = remoteIPStr.c_str();
				bool found = false;
				WiFiClientNode *c = conns;
				while (c != nullptr)
				{
					if ((c->isConnected()) && (c->port == port) && (strcmp(remoteIP, c->host) == 0))
						found = true;
					c = c->next;
				}
				if (!found)
					new WiFiClientNode(newClient, serv->flagsBitmap, 5); // constructing is enough
																		 // else // auto disconnect when from same
			}
		}
		serv = serv->next;
	}

	WiFiClientNode *conn = conns;
	unsigned long now = millis();
	while (conn != nullptr)
	{
		WiFiClientNode *nextConn = conn->next;
		if ((!conn->isAnswered()) && (conn->isConnected()) && (conn != current) && (!conn->isMarkedForDisconnect()))
		{
			conn->write((uint8_t *)busyMsg.c_str(), busyMsg.length());
			conn->flushAlways();
			conn->markForDisconnect();
		}
		conn = nextConn;
	}

	WiFiClientNode::checkForAutoDisconnections();

	if (lastDTR == dtrActive)
	{
		lastDTR = digitalRead(PIN_DTR);
		if ((lastDTR == dtrInactive) && (dtrInactive != dtrActive))
		{
			if (current != nullptr)
				current->setDisconnectOnStreamExit(true);
			switchBackToCommandMode(true);
		}
	}
	lastDTR = digitalRead(PIN_DTR);
	if ((current == nullptr) || (!current->isConnected()))
	{
		switchBackToCommandMode(true);
	}
	else if ((currentExpiresTimeMs > 0) && (millis() > currentExpiresTimeMs))
	{
		currentExpiresTimeMs = 0;
		if (plussesInARow == 3)
		{
			plussesInARow = 0;
			if (current != 0)
			{
				CommandMode.sendOfficialResponse(ZOK);
				switchBackToCommandMode(false);
			}
		}
	}
	else if (serial.isSerialOut())
	{
		if (current->available() > 0)
		//&&(current->isConnected()) // not a requirement to have available bytes to read
		{
			int bufferRemaining = serialOutBufferBytesRemaining();
			if (bufferRemaining > 0)
			{
				int bytesAvailable = current->available();
				if (bytesAvailable > bufferRemaining)
					bytesAvailable = bufferRemaining;
				if (bytesAvailable > 0)
				{
					for (int i = 0; (i < bytesAvailable) && (current->available() > 0); i++)
					{
						if (serial.isSerialCancelled())
							break;
						uint8_t c = current->read();
						logSocketIn(c);
						if ((!isTelnet() || handleAsciiIAC((char *)&c, current)) && (!isPETSCII() || ascToPet((char *)&c, current)))
							serial.printb(c);
					}
				}
			}
		}
		if (serial.isSerialOut())
		{
			if ((nextFlushMs > 0) && (millis() > nextFlushMs))
			{
				nextFlushMs = 0;
				serial.flush();
			}
			serialOutDeque();
		}
	}
	checkBaudChange();
}

StreamHandler StreamMode;