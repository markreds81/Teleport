#ifndef CONSOLE_H
#define CONSOLE_H

#include "Profile.h"
#include <Arduino.h>

#define ZCONSOLE_SHOW_MENU 0x01
#define ZCONSOLE_DONE 0x02
#define ZCONSOLE_CHANGED 0x04

class Console
{
private:
	enum MenuScreen
	{
		ZMENU_MAIN = 0,
		ZMENU_NUM = 1,
		ZMENU_ADDRESS = 2,
		ZMENU_OPTIONS = 3,
		ZMENU_WIMENU = 4,
		ZMENU_WIFI_PASS = 5,
		ZMENU_WICONFIRM = 6,
		ZMENU_FLOW = 7,
		ZMENU_BBSMENU = 8,
		ZMENU_NEWPORT = 9,
		ZMENU_HOST = 10,
		ZMENU_NOTES = 11,
		ZMENU_NETMENU = 12,
		ZMENU_SUBNET = 13
	} menu;
	uint8_t state;
	char EOLN[3];

public:
	Console();
	virtual ~Console();

	void begin(Profile &profile);
	void exec(String cmd);
	bool done(Profile &profile);
	void end();
};

#endif