#include "ZSerial.h"
#include "ZModem.h"
#include "ZDebug.h"
#include "Button.h"
#include <Arduino.h>

Button resetButton(PIN_FACTORY_RESET);
ZModem modem(SerialPort);

void setup()
{
#if DEBUG
	DebugPort.begin();
#endif
	
	resetButton.begin();
	modem.begin();	
}

void loop()
{
	if (resetButton.pressed())
	{
		modem.factoryReset();
	}

	modem.tick();
}