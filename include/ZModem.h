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

class ZModem
{
private:
	HardwareSerial &serialPort;
	uint8_t buffer[MAX_COMMAND_SIZE];
	size_t buflen;
	char ECS[32];
	bool doEcho;
	unsigned long lastNonPlusTimeMs = 0;
	unsigned long currentExpiresTimeMs = 0;

	bool readSerialStream();
	void clearPlusProgress();
	ZResult doSerialCommand();

public:
	ZModem(HardwareSerial serial);
	virtual ~ZModem();

	void factoryReset();

	void begin();
	void tick();
};

#endif