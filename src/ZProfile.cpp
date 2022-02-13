#include "ZProfile.h"
#include "ZDebug.h"
#include "z/options.h"
#include "z/version.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

ZProfile::ZProfile()
{
}

ZProfile::~ZProfile()
{
}

void ZProfile::begin(int num)
{
	if (num < 0 || num > 1 || !loadProfile(num))
	{
		loadFactoryProfile();
	}
}

void ZProfile::loadFactoryProfile()
{
	memset(regs, 0, sizeof(regs));
	memset(hostname, 0, sizeof(hostname));
	memset(wifiSSID, 0, sizeof(wifiSSID));
	memset(wifiPSWD, 0, sizeof(wifiPSWD));
	regs[2] = (uint8_t)'+';
	regs[3] = (uint8_t)'\r';		// ASCII 13
	regs[4] = (uint8_t)'\n';		// ASCII 10
	regs[5] = (uint8_t)'\b';		// ASCII 8
	regs[12] = 50;					// in fiftieths of a second
	regs[14] = 0b00001010;
	regs[32] = ASCII_XON;
	regs[33] = ASCII_XOFF;
	baudRate = DEFAULT_BAUD_RATE;
}

bool ZProfile::loadProfile(int num)
{
	char name[32];
	snprintf(name, sizeof(name), "/profile/%d", num);
	File file = SPIFFS.open(name, "r");
	bool result = false;
	if (file)
	{
		StaticJsonDocument<512> doc;
		if (deserializeJson(doc, file))
		{
			// strcpy(hostname, doc["hostname"]);
			// strcpy(wifiSSID, doc["wifiSSID"]);
			// strcpy(wifiPSWD, doc["wifiPSWD"]);
			// baudRate = doc["baudRate"];
			// JsonArray array = doc["regs"].as<JsonArray>();
			// for (int i = 0; i < sizeof(regs) && i < array.size(); i++)
			// {
			// 	regs[i] = array[i].as<uint8_t>();
			// }
			// result = true;
		}
		file.close();
	}
	return result;
}

bool ZProfile::saveProfile(int num)
{
	char name[32];
	snprintf(name, sizeof(name), "/profile/%d", num);
	File file = SPIFFS.open(name, "w");
	bool result = false;
	if (file)
	{
		StaticJsonDocument<1024> doc;

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

		result = (serializeJson(doc, file) > 0);
		file.close();
	}
	return result;
}
