#include "ZModem.h"
#include "ZDebug.h"
#include <Arduino.h>
#include <Button.h>

const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 1 * 3600;	// GMT+1 Rome, Italy
const int DST_OFFSET_SEC = 1 * 3600;	// DST off 60' Italy

Button reset(PIN_FACTORY_RESET);
ZModem modem;

void setup()
{
#if DEBUG
	Serial.begin();
#endif

	configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);

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