#ifndef ZMODEM_H
#define ZMODEM_H

#define PIN_FACTORY_RESET GPIO_NUM_0
#define PIN_LED_HS GPIO_NUM_2
#define PIN_LED_WIFI GPIO_NUM_22
#define PIN_LED_DATA GPIO_NUM_33
#define PIN_DCD GPIO_NUM_14
#define PIN_CTS GPIO_NUM_13
#define PIN_RTS GPIO_NUM_15 // unused
#define PIN_RI GPIO_NUM_32
#define PIN_DSR GPIO_NUM_12
#define PIN_DTR GPIO_NUM_27

#define DEFAULT_BAUD_RATE 1200
#define DEFAULT_SERIAL_CONFIG SERIAL_8N1
#define MAX_COMMAND_SIZE 256

#define ASCII_BS 8
#define ASCII_XON 17
#define ASCII_XOFF 19
#define ASCII_DC4 20
#define ASCII_DELETE 127

#include <Arduino.h>

enum ZResult
{
	ZOK,
	ZERROR,
	ZCONNECT,
	ZNOCARRIER,
	ZNOANSWER,
	ZIGNORE,
	ZIGNORE_SPECIAL
};

enum FlowControlType
{
	FCT_RTSCTS = 0,
	FCT_NORMAL = 1,
	FCT_AUTOOFF = 2,
	FCT_MANUAL = 3,
	FCT_DISABLED = 4,
	FCT_INVALID = 5
};

class ZModem
{
private:
	HardwareSerial &serialPort;
	uint8_t buffer[MAX_COMMAND_SIZE];
	size_t buflen;
	char CR;
	char LF;
	char EC;
	char BS;
	char ECS[32];
	bool echoActive;
	bool numericResponses;
	unsigned long lastNonPlusTimeMs = 0;
	unsigned long currentExpiresTimeMs = 0;
	FlowControlType flowControlType;

	char lc(char c);
	bool readSerialStream();
	void clearPlusProgress();
	void showResponse(ZResult rc);

	ZResult doSerialCommand();

public:
	ZModem(HardwareSerial &serial);
	virtual ~ZModem();

	void factoryReset();

	void begin();
	void tick();
};

#endif