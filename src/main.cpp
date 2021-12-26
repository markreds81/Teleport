#include "ZSerial.h"
#include "ZModem.h"
#include "ZDebug.h"
#include "Button.h"
#include "driver/uart.h"
#include <Arduino.h>

Button reset(PIN_FACTORY_RESET);
ZSerial serial(UART_NUM_2);
ZModem modem(serial);

void setup()
{
#if DEBUG
	DebugPort.begin();
#endif
	
	reset.begin();
	modem.begin();	
}

void loop()
{
	if (reset.pressed())
	{
		modem.factoryReset();
	}

	modem.tick();
}