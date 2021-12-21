#include <Arduino.h>
#include "debug.h"
#include "ZModem.h"
#include "Button.h"

Button resetButton(PIN_FACTORY_RESET);
ZModem modem(Serial2);

void setup()
{
#if DEBUG
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	Serial.println("Debug port open and ready.");
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