#include "ZConsole.h"
#include "ZSerial.h"
#include "ZDebug.h"
#include "ZSettings.h"
#include "ZPhonebook.h"
#include <WiFi.h>

ZConsole::ZConsole()
{
}

ZConsole::~ZConsole()
{
}

void ZConsole::begin()
{
	state = ZCONSOLE_SHOW_MENU;
	menu = ZMENU_MAIN;
}

void ZConsole::exec(String cmd)
{
	char c = '?';
	for (int i = 0; i < cmd.length(); i++)
	{
		if (cmd[i] > 32)
		{
			c = tolower(cmd[i]);
			break;
		}
	}
	switch (menu)
	{
	case ZMENU_MAIN:
		if (c == 'q' || cmd.length() == 0)
		{
			if (state & ZCONSOLE_CHANGED)
			{
				menu = ZMENU_WICONFIRM;
				state |= ZCONSOLE_SHOW_MENU;
			}
			else
			{
				state = ZCONSOLE_DONE;
			}
		}
		else if (c == 'a') // add to phonebook
		{
		}
		else if (c == 'w') // wifi
		{
		}
		else if (c == 'h') // host
		{
		}
		else if (c == 'f') // flow control
		{
		}
		else if (c == 'p') // PETSCII traslation toggle
		{
		}
		else if (c == 'e') // echo toggle
		{
		}
		else if (c == 'b') // bbs
		{
		}
		else if (c > 47 && c < 58) // phonebook entry
		{
		}
		else
		{
			state |= ZCONSOLE_SHOW_MENU;
		}
		break;
	default:
		DPRINTF("Current menu: %d\n", menu);
	}
}

bool ZConsole::done()
{
	if (state & ZCONSOLE_SHOW_MENU)
	{
		state &= ~ZCONSOLE_SHOW_MENU;

		switch (menu)
		{
		case ZMENU_MAIN:
			Serial2.printf("%sMain Menu%s", Settings.EOLN.c_str(), Settings.EOLN.c_str());
			Serial2.printf("[HOST] name: %s%s", Settings.hostname.c_str(), Settings.EOLN.c_str());
			Serial2.printf("[WIFI] connection: %s%s", (WiFi.status() == WL_CONNECTED) ? Settings.wifiSSID.c_str() : "Not connected", Settings.EOLN.c_str());
			// Serial2.printf("[FLOW] control: %s%s", Settings.flowName.c_str(), Settings.EOLN.c_str());
			Serial2.printf("[ECHO] keystrokes: %s%s", Settings.doEcho ? "ON" : "OFF", Settings.EOLN.c_str());
			// serial.printf("[BBS] host: %s%s",bbsMode.c_str(),EOLNC);
			// Serial2.printf("[PETSCII] translation: %s%s", Serial2.isPetsciiMode()?"ON":"OFF", Settings.EOLN.c_str());
			Serial2.printf("[ADD] new phonebook entry%s", Settings.EOLN.c_str());
			if (!Phonebook.empty())
			{
				PBEntry pbe;
				Serial2.printf("Phonebook entries:%s", Settings.EOLN.c_str());
				for (int i = 0; i < Phonebook.size(); i++)
				{
					Phonebook.get(i, &pbe);
					if (strlen(pbe.notes))
						Serial2.printf("  [%lu] %s (%s)%s", pbe.number, pbe.address, pbe.notes, Settings.EOLN.c_str());
					else
						Serial2.printf("  [%lu] %s%s", pbe.number, pbe.address, Settings.EOLN.c_str());
				}
			}
			Serial2.printf("%sEnter command or entry or ENTER to exit: ", Settings.EOLN.c_str());
			break;
		default:
			DPRINTF("Show menu: %d\n", menu);
		}
	}
	return (state & ZCONSOLE_DONE) == ZCONSOLE_DONE;
}

void ZConsole::end()
{
}