#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

#define ZIMODEM_VERSION					"3.6.5"
#define ZIMODEM_ESP32

#define PIN_FACTORY_RESET 				GPIO_NUM_0
#define PIN_LED_HS 						GPIO_NUM_2
#define PIN_LED_WIFI 					GPIO_NUM_22
#define PIN_LED_DATA 					GPIO_NUM_33
#define PIN_DCD							GPIO_NUM_14
#define PIN_CTS							GPIO_NUM_13
#define PIN_RTS							GPIO_NUM_15 // unused
#define PIN_RI							GPIO_NUM_32
#define PIN_DSR							GPIO_NUM_12
#define PIN_DTR							GPIO_NUM_27

#define DEFAULT_NO_DELAY 				true
#define DEFAULT_BAUD_RATE 				1200
#define DEFAULT_HS_BAUD					38400
#define DEFAULT_SERIAL_CONFIG 			SERIAL_8N1
#define DEFAULT_FCT						FCT_DISABLED
#define INTERNAL_FLOW_CONTROL_DIV		380
#define ENC_TYPE_NONE					WIFI_AUTH_OPEN

#define SerialDebug 					Serial
#define preEOLN							serial.prints
#define echoEOLN						serial.write

typedef uint32_t SerialConfig;

const char compile_date[] = __DATE__ " " __TIME__;

enum ZResult {
	ZOK,
	ZERROR,
	ZCONNECT,
	ZNOCARRIER,
	ZNOANSWER,
	ZIGNORE,
	ZIGNORE_SPECIAL
};

enum FlowControlType {
	FCT_RTSCTS = 0,
	FCT_NORMAL = 1,
	FCT_AUTOOFF = 2,
	FCT_MANUAL = 3,
	FCT_DISABLED = 4,
	FCT_INVALID = 5
};

enum BaudState {
	BS_NORMAL,
	BS_SWITCH_TEMP_NEXT,
	BS_SWITCHED_TEMP,
	BS_SWITCH_NORMAL_NEXT
};

class ZMode {
public:
	virtual void serialIncoming();
	virtual void loop();
};

extern HardwareSerial SerialDTE;
extern ZMode *currentMode;
extern int wifiNextNodeId;
extern bool wifiConnected;
extern SerialConfig serialConfig;
extern String wifiSSI;
extern String wifiPWD;
extern String hostname;
extern IPAddress *staticIP;
extern IPAddress *staticDNS;
extern IPAddress *staticGW;
extern IPAddress *staticSN;
extern int dequeSize;
extern int ctsActive;
extern int ctsInactive;
extern int rtsActive;
extern int rtsInactive;
extern int dcdStatus;
extern int dcdActive;
extern int dcdInactive;
extern int riActive;
extern int riInactive;
extern int dtrActive;
extern int dtrInactive;
extern int dsrActive;
extern int dsrInactive;
extern int tempBaud;
extern int baudRate;
extern String termType;
extern String busyMsg;

void changeSerialConfig(SerialConfig conf);
void setNewStaticIPs(IPAddress *ip, IPAddress *dns, IPAddress *gateWay, IPAddress *subNet);
bool connectWifi(const char *ssid, const char *password, IPAddress *ip, IPAddress *dns, IPAddress *gateWay, IPAddress *subNet);
int checkOpenConnections();
void checkBaudChange();
void changeBaudRate(int baudRate);

#endif