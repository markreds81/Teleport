#include "ZProfile.h"
#include "ZDebug.h"
#include "z/options.h"
#include "z/version.h"
#include <ArduinoJson.h>

ZProfile::ZProfile()
{
	reset();
}

ZProfile::~ZProfile()
{
}

void ZProfile::reset()
{
	memset(regs, 0, sizeof(regs));
	regs[2] = (uint8_t)'+';
	regs[3] = (uint8_t)'\r';		// ASCII 13
	regs[4] = (uint8_t)'\n';		// ASCII 10
	regs[5] = (uint8_t)'\b';		// ASCII 8
	regs[12] = 50;					// in fiftieths of a second
	regs[14] = 0b00001110;
	regs[32] = ASCII_XON;
	regs[33] = ASCII_XOFF;
	baudRate = DEFAULT_BAUD_RATE;
}

void ZProfile::loadProfile(int num)
{
	// bool result = false;

	// if (num >= 0)
	// {
	// 	char name[32];
	// 	snprintf(name, sizeof(name), "/profile/%d", num);
	// 	File file = SPIFFS.open(name, "r");
	// 	if (file)
	// 	{
	// 		uint8_t *buffer = reinterpret_cast<uint8_t *>(profile);
	// 		if (file.read(buffer, sizeof(ZProfile)) == sizeof(ZProfile))
	// 		{
	// 			result = true;
	// 			DPRINTF("Profile %d %s\n", num, "loaded");
	// 		}
	// 		file.close();
	// 	}
	// }

	// if (num < 0 || !result)
	// {
	// 	reset();
	// }

	// return result;
}

void ZProfile::saveProfile(int num)
{
	DynamicJsonDocument doc(1024);

	doc["version"] = ZMODEM_VERSION;
	doc["hostname"] = hostname;
	doc["wifiSSID"] = wifiSSID;
	doc["wifiPSWD"] = wifiPSWD;
	doc["baudRate"] = baudRate;

	JsonArray array = doc.createNestedArray("regs");

	for (int i = 0; i < sizeof(regs); i++)
	{
		array.add(regs[i]);
	}

	serializeJson(doc, Serial);

	// bool result = false;

	// if (num >= 0)
	// {
	// 	char name[32];
	// 	snprintf(name, sizeof(name), "/profile/%d", num);
	// 	File file = SPIFFS.open(name, "w");
	// 	if (file)
	// 	{
	// 		uint8_t *buffer = reinterpret_cast<uint8_t *>(profile);
	// 		if (file.write(buffer, sizeof(ZProfile)) == sizeof(ZProfile))
	// 		{
	// 			result = true;
	// 			DPRINTF("Profile %d %s\n", num, "saved");
	// 		}
	// 		file.close();
	// 	}
	// }

	// return result;
}
