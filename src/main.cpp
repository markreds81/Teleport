#include "main.h"
#include "driver/uart.h"
#include "Button.h"
#include "SerialBuffer.h"
#include "WiFiClientNode.h"
#include "CommandMode.h"
#include "Logger.h"
#include "SerialBuffer.h"
#include "PhoneBookEntry.h"
#include "RTClock.h"

HardwareSerial SerialDTE(UART_NUM_2);
ZMode *currentMode;
Button resetButton(PIN_FACTORY_RESET);
int wifiNextNodeId = 0;
bool wifiConnected = false;
SerialConfig serialConfig = DEFAULT_SERIAL_CONFIG;
String wifiSSI;
String wifiPWD;
String hostname;
IPAddress *staticIP = nullptr;
IPAddress *staticDNS = nullptr;
IPAddress *staticGW = nullptr;
IPAddress *staticSN = nullptr;
int tempBaud = -1;
int baudRate = DEFAULT_BAUD_RATE;
int dcdStatus = LOW;
int dcdActive = HIGH;
int dcdInactive = LOW;
int ctsActive = HIGH;
int ctsInactive = LOW;
int rtsActive = HIGH;
int rtsInactive = LOW;
int riActive = HIGH;
int riInactive = LOW;
int dtrActive = HIGH;
int dtrInactive = LOW;
int dsrActive = HIGH;
int dsrInactive = LOW;
BaudState baudState = BS_NORMAL;
int dequeSize = 1 + (DEFAULT_BAUD_RATE / INTERNAL_FLOW_CONTROL_DIV);
String termType = DEFAULT_TERMTYPE;
String busyMsg = DEFAULT_BUSYMSG;

void changeSerialConfig(SerialConfig conf)
{
	flushSerial(); // blocking, but very very necessary
	delay(500);	   // give the client half a sec to catch up
	SerialDebug.printf("Config changing %d.\n", (int)conf);
	dequeSize = 1 + (baudRate / INTERNAL_FLOW_CONTROL_DIV);
	SerialDebug.printf("Deque constant now: %d\n", dequeSize);
	// SerialDTE.changeConfig(conf);
	SerialDebug.println("changeConfig() not implemented");
	SerialDebug.printf("Config changed.\n");
}

void setNewStaticIPs(IPAddress *ip, IPAddress *dns, IPAddress *gateWay, IPAddress *subNet)
{
	if (staticIP != nullptr)
		free(staticIP);
	staticIP = ip;
	if (staticDNS != nullptr)
		free(staticDNS);
	staticDNS = dns;
	if (staticGW != nullptr)
		free(staticGW);
	staticGW = gateWay;
	if (staticSN != nullptr)
		free(staticSN);
	staticSN = subNet;
}

void setHostName(const char *hname)
{
	tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, hname);
}

bool connectWifi(const char *ssid, const char *password, IPAddress *ip, IPAddress *dns, IPAddress *gateWay, IPAddress *subNet)
{
	while (WiFi.status() == WL_CONNECTED)
	{
		WiFi.disconnect();
		delay(100);
		yield();
	}
#ifndef ZIMODEM_ESP32
	if (hostname.length() > 0)
		setHostName(hostname.c_str());
#endif
	WiFi.mode(WIFI_STA);
	if ((ip != nullptr) && (gateWay != nullptr) && (dns != nullptr) && (subNet != nullptr))
	{
		if (!WiFi.config(*ip, *gateWay, *subNet, *dns))
			return false;
	}
	WiFi.begin(ssid, password);
	if (hostname.length() > 0)
		setHostName(hostname.c_str());
	bool amConnected = (WiFi.status() == WL_CONNECTED) && (strcmp(WiFi.localIP().toString().c_str(), "0.0.0.0") != 0);
	int WiFiCounter = 0;
	while ((!amConnected) && (WiFiCounter < 30))
	{
		WiFiCounter++;
		if (!amConnected)
			delay(500);
		amConnected = (WiFi.status() == WL_CONNECTED) && (strcmp(WiFi.localIP().toString().c_str(), "0.0.0.0") != 0);
	}
	wifiConnected = amConnected;
	if (!amConnected)
		WiFi.disconnect();
#ifdef SUPPORT_LED_PINS
	s_pinWrite(DEFAULT_PIN_WIFI, wifiConnected ? DEFAULT_WIFI_ACTIVE : DEFAULT_WIFI_INACTIVE);
#endif
	return wifiConnected;
}

int checkOpenConnections()
{
	int num = WiFiClientNode::getNumOpenWiFiConnections();
	if (num == 0)
	{
		if ((dcdStatus == dcdActive) && (dcdStatus != dcdInactive))
		{
			SerialDebug.println("DCD going inactive.");
			dcdStatus = dcdInactive;
			digitalWrite(PIN_DCD, dcdStatus);
			if (baudState == BS_SWITCHED_TEMP)
			{
				baudState = BS_SWITCH_NORMAL_NEXT;
			}
			if (currentMode == &commandMode)
			{
				clearSerialOutBuffer();
			}
		}
	}
	else
	{
		if ((dcdStatus == dcdInactive) && (dcdStatus != dcdActive))
		{
			SerialDebug.println("DCD going active.");
			dcdStatus = dcdActive;
			digitalWrite(PIN_DCD, dcdStatus);
			if ((tempBaud > 0) && (baudState == BS_NORMAL))
			{
				baudState = BS_SWITCH_TEMP_NEXT;
			}
		}
	}
	return num;
}

void factoryReset()
{
	SerialDTE.flush();
	delay(500);
	SerialDTE.end();

	SerialDebug.println("reset");
	SerialDebug.end();

	ESP.restart();
}

void checkBaudChange()
{
	switch (baudState)
	{
	case BS_SWITCH_TEMP_NEXT:
		changeBaudRate(tempBaud);
		baudState = BS_SWITCHED_TEMP;
		break;
	case BS_SWITCH_NORMAL_NEXT:
		changeBaudRate(baudRate);
		baudState = BS_NORMAL;
		break;
	default:
		break;
	}
}

void changeBaudRate(int baudRate)
{
	flushSerial(); // blocking, but very very necessary
	delay(500);	   // give the client half a sec to catch up
	logPrintfln("Baud change to %d.\n", baudRate);
	SerialDebug.printf("Baud change to %d.\n", baudRate);
	dequeSize = 1 + (baudRate / INTERNAL_FLOW_CONTROL_DIV);
	SerialDebug.printf("Deque constant now: %d\n", dequeSize);
	// SerialDTE.changeBaudRate(baudRate);
	SerialDebug.println("SerialDTE.changeBaudRate not implemented");
	digitalWrite(PIN_LED_HS, (baudRate >= DEFAULT_HS_BAUD) ? HIGH : LOW);
}

void setup()
{
	SerialDebug.begin(115200);
	SerialDebug.setDebugOutput(true);
	SerialDebug.println("Debug port open and ready.");

	pinMode(PIN_LED_HS, OUTPUT);
	pinMode(PIN_LED_DATA, OUTPUT);
	pinMode(PIN_LED_WIFI, OUTPUT);

	resetButton.begin();

	digitalWrite(PIN_LED_DATA, HIGH);
	delay(200);
	digitalWrite(PIN_LED_DATA, LOW);

	digitalWrite(PIN_LED_HS, HIGH);
	delay(200);
	digitalWrite(PIN_LED_HS, LOW);

	digitalWrite(PIN_LED_WIFI, HIGH);
	delay(200);
	digitalWrite(PIN_LED_WIFI, LOW);

	SerialDTE.begin(DEFAULT_BAUD_RATE, DEFAULT_SERIAL_CONFIG);
	// initSDShell();
	if (!SPIFFS.begin())
	{
		SPIFFS.format();
		SPIFFS.begin();
		SerialDebug.println("SPIFFS Formatted.");
	}

	currentMode = &commandMode;
	commandMode.loadConfig();
	PhoneBookEntry::loadPhonebook();
	dcdStatus = dcdInactive;
	digitalWrite(PIN_DCD, dcdStatus);
	digitalWrite(PIN_LED_WIFI, (WiFi.status() == WL_CONNECTED) ? HIGH : LOW);
}

void loop()
{
	if (resetButton.pressed())
	{
		factoryReset();
	}

	if (SerialDTE.available() > 0)
	{
		currentMode->serialIncoming();
	}

	currentMode->loop();

	zclock.tick();
}