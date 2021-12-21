#ifndef WI_FI_CLIENT_NODE_H
#define WI_FI_CLIENT_NODE_H

#define LAST_PACKET_BUF_SIZE                256
#define OVERFLOW_BUF_SIZE                   256
#define UNDERFLOW_BUF_MAX_SIZE              256

#include "main.h"
#include "z_types.h"
#include <WiFiClientSecure.h>

WiFiClient *createWiFiClient(bool SSL);

class WiFiClientNode : public Stream {
public:
    WiFiClientNode(char *hostIp, int newport, int flagsBitmap);
    WiFiClientNode(WiFiClient newClient, int flagsBitmap, int ringDelay);
    ~WiFiClientNode();

    int id=0;
    char *host;
    int port;
    bool wasConnected=false;
    bool serverClient=false;
    int flagsBitmap = 0;
    char *delimiters = NULL;
    char *maskOuts = NULL;
    char *stateMachine = NULL;
    char *machineState = NULL;
    String machineQue = "";
    uint8_t lastPacketBuf[LAST_PACKET_BUF_SIZE];
    int lastPacketLen=0;
    //uint8_t overflowBuf[OVERFLOW_BUF_SIZE];
    //int overflowBufLen = 0;
    uint8_t underflowBuf[UNDERFLOW_BUF_MAX_SIZE];
    size_t underflowBufLen = 0;
    WiFiClientNode *next = nullptr;

    bool isConnected();
    FlowControlType getFlowControl();
    bool isPETSCII();
    bool isEcho();
    bool isTelnet();

    bool isAnswered();
    void answer();
    int ringsRemaining(int delta);
    unsigned long nextRingTime(long delta);
    void markForDisconnect();
    bool isMarkedForDisconnect();

    bool isDisconnectedOnStreamExit();
    void setDisconnectOnStreamExit(bool tf);

    void setNoDelay(bool tf);

    size_t write(uint8_t c);
    size_t write(const uint8_t *buf, size_t size);
    int read();
    int peek();
    void flush();
    void flushAlways();
    int available();
    int read(uint8_t *buf, size_t size);
    String readLine(unsigned int timeout);

    static int getNumOpenWiFiConnections();
    static int checkForAutoDisconnections();
private:
    WiFiClient client;
    WiFiClient *clientPtr;
    bool answered=true;
    int ringsRemain=0;
    unsigned long nextRingMillis = 0;
    unsigned long nextDisconnect = 0;

    void finishConnectionLink();
    int flushOverflowBuffer();
    void fillUnderflowBuf();
};

extern WiFiClientNode *conns;

#endif