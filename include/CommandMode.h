#ifndef COMMAND_MODE_H
#define COMMAND_MODE_H

#include "main.h"
#include "z_types.h"
#include "WiFiClientNode.h"
#include "SerialBuffer.h"

#define DEFAULT_TERMTYPE				"Zimodem"
#define DEFAULT_BUSYMSG					"\r\nBUSY\r\n7\r\n"
#define ZI_STATE_MACHINE_LEN			7
#define MAX_COMMAND_SIZE                256

const ConfigOptions v2HexCfgs[] = { CFG_WIFISSI, CFG_WIFIPW, CFG_TIMEZONE, CFG_TIMEFMT, CFG_TIMEURL,
    CFG_PRINTSPEC, CFG_BUSYMSG, CFG_HOSTNAME, CFG_TERMTYPE, (ConfigOptions)255 };

enum BinType {
	BTYPE_NORMAL = 0,
	BTYPE_HEX = 1,
	BTYPE_DEC = 2,
	BTYPE_NORMAL_NOCHK = 3,
	BTYPE_INVALID = 4
};

class CommandMode : public ZMode {

friend class WiFiClientNode;
friend class ZConfig;

private:
	char CRLF[4];
    char LFCR[4];
    char LF[2];
    char CR[2];
    char BS=8;
    char ringCounter = 1;
	ZSerial serial;
    bool packetXOn = true;
    BinType binType = BTYPE_NORMAL;
    uint8_t nbuf[MAX_COMMAND_SIZE];
    char hbuf[MAX_COMMAND_SIZE];
    int eon=0;
    int lastServerClientId = 0;
	bool autoStreamMode=false;
    bool preserveListeners=false;
    unsigned long lastNonPlusTimeMs = 0;
    unsigned long currentExpiresTimeMs = 0;
    char *tempDelimiters = NULL;
    char *tempMaskOuts = NULL;
    char *tempStateMachine = NULL;
    char *delimiters = NULL;
    char *maskOuts = NULL;
    char *stateMachine = NULL;
    char *machineState = NULL;
    String machineQue = "";
    String previousCommand = "";
	int lastPacketId = -1;
	WiFiClientNode *current = nullptr;
	WiFiClientNode *nextConn = nullptr;

	byte CRC8(const byte *data, byte len);
	void showInitMessage();
    bool readSerialStream();
    bool clearPlusProgress();
    bool checkPlusEscape();
    String getNextSerialCommand();
    ZResult doSerialCommand();
    void setConfigDefaults();
    void parseConfigOptions(String configArguments[]);
    void setOptionsFromSavedConfig(String configArguments[]);
    void reSaveConfig();
    void reSendLastPacket(WiFiClientNode *conn);
    void acceptNewConnection();
    void headerOut(const int channel, const int sz, const int crc8);
    void sendConnectionNotice(int nodeId);
    void sendNextPacket();
    void connectionArgs(WiFiClientNode *c);
    void updateAutoAnswer();
    uint8_t *doStateMachine(uint8_t *buf, int *bufLen, char **machineState, String *machineQue, char *stateMachine);
    uint8_t *doMaskOuts(uint8_t *buf, int *bufLen, char *maskOuts);
    ZResult doWebDump(Stream *in, int len, const bool cacheFlag);
    ZResult doWebDump(const char *filename, const bool cache);

    ZResult doResetCommand();
    ZResult doNoListenCommand();
    ZResult doBaudCommand(int vval, uint8_t *vbuf, int vlen);
    ZResult doTransmitCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers, int *crc8);
    ZResult doLastPacket(int vval, uint8_t *vbuf, int vlen, bool isNumber);
    ZResult doConnectCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
    ZResult doWiFiCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
    ZResult doDialStreamCommand(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
    ZResult doPhonebookCommand(unsigned long vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
    ZResult doAnswerCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *dmodifiers);
    ZResult doHangupCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber);
    ZResult doEOLNCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber);
    ZResult doInfoCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber);
    ZResult doWebStream(int vval, uint8_t *vbuf, int vlen, bool isNumber, const char *filename, bool cache);
    ZResult doUpdateFirmware(int vval, uint8_t *vbuf, int vlen, bool isNumber);
    ZResult doTimeZoneSetupCommand(int vval, uint8_t *vbuf, int vlen, bool isNumber);
public:
	int packetSize = 127;
    bool suppressResponses;
    bool numericResponses;
    bool longResponses;
    boolean doEcho;
    String EOLN;
    char EC='+';
    char ECS[32];

    CommandMode();
    virtual ~CommandMode();

	void loadConfig();
	FlowControlType getFlowControlType();
    int getConfigFlagBitmap();
	void sendOfficialResponse(ZResult res);
    void serialIncoming();
    void loop();
	void reset();
};

extern CommandMode commandMode;

#endif