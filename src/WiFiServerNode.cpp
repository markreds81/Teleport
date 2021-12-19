#include "WiFiServerNode.h"
#include "AsciiUtils.h"

WiFiServerSpec::WiFiServerSpec()
{
    setCharArray(&delimiters, "");
    setCharArray(&maskOuts, "");
    setCharArray(&stateMachine, "");
}

WiFiServerSpec::~WiFiServerSpec()
{
    freeCharArray(&delimiters);
    freeCharArray(&maskOuts);
    freeCharArray(&stateMachine);
}

WiFiServerSpec::WiFiServerSpec(WiFiServerSpec &copy)
{
    port = copy.port;
    flagsBitmap = copy.flagsBitmap;
    setCharArray(&delimiters, copy.delimiters);
    setCharArray(&maskOuts, copy.maskOuts);
    setCharArray(&stateMachine, copy.stateMachine);
}

WiFiServerSpec &WiFiServerSpec::operator=(const WiFiServerSpec &copy)
{
    if (this != &copy)
    {
        port = copy.port;
        flagsBitmap = copy.flagsBitmap;
        setCharArray(&delimiters, copy.delimiters);
        setCharArray(&maskOuts, copy.maskOuts);
        setCharArray(&stateMachine, copy.stateMachine);
    }
    return *this;
}

WiFiServerNode::WiFiServerNode(int newport, int flagsBitmap)
{
    id = ++wifiNextNodeId;
    port = newport;
    this->flagsBitmap = flagsBitmap;
    server = new WiFiServer(newport);
    // BZ:server->setNoDelay(DEFAULT_NO_DELAY);
    server->begin();
    if (servs == nullptr)
        servs = this;
    else
    {
        WiFiServerNode *s = servs;
        while (s->next != nullptr)
            s = s->next;
        s->next = this;
    }
}

WiFiServerNode::~WiFiServerNode()
{
    if (server != nullptr)
    {
        server->stop();
        server->close();
        delete server;
    }
    if (servs == this)
        servs = next;
    else
    {
        WiFiServerNode *last = servs;
        while ((last != nullptr) && (last->next != this)) // don't change this!
            last = last->next;
        if (last != nullptr)
            last->next = next;
    }
}

bool WiFiServerNode::hasClient()
{
    if (server != nullptr)
        return server->hasClient();
    return false;
}

bool WiFiServerNode::ReadWiFiServer(File &f, WiFiServerSpec &node)
{
    if (f.available() > 0)
    {
        String str = "";
        char c = f.read();
        while ((c != ',') && (f.available() > 0))
        {
            str += c;
            c = f.read();
        }
        if (str.length() == 0)
            return false;
        node.port = atoi(str.c_str());
        str = "";
        c = f.read();
        while ((c != '\n') && (f.available() > 0))
        {
            str += c;
            c = f.read();
        }
        if (str.length() == 0)
            return false;
        node.flagsBitmap = atoi(str.c_str());
        str = "";
        c = f.read();
        while ((c != ',') && (f.available() > 0))
        {
            str += c;
            c = f.read();
        }
        if (str.length() == 0)
            return false;
        int chars = atoi(str.c_str());
        str = "";
        for (int i = 0; i < chars && f.available() > 0; i++)
        {
            c = f.read();
            str += c;
        }
        setCharArray(&node.maskOuts, str.c_str());
        if (f.available() <= 0 || f.read() != '\n')
            return false;
        str = "";
        c = f.read();
        while ((c != ',') && (f.available() > 0))
        {
            str += c;
            c = f.read();
        }
        if (str.length() == 0)
            return false;
        chars = atoi(str.c_str());
        str = "";
        for (int i = 0; i < chars && f.available() > 0; i++)
        {
            c = f.read();
            str += c;
        }
        setCharArray(&node.delimiters, str.c_str());
        if (f.available() <= 0 || f.read() != '\n')
            return true;
        str = "";
        c = f.read();
        while ((c != ',') && (f.available() > 0))
        {
            str += c;
            c = f.read();
        }
        if (str.length() == 0)
            return false;
        chars = atoi(str.c_str());
        str = "";
        for (int i = 0; i < chars && f.available() > 0; i++)
        {
            str += c;
            c = f.read();
        }
        setCharArray(&node.stateMachine, str.c_str());
    }
    return true;
}

void WiFiServerNode::SaveWiFiServers()
{
    SPIFFS.remove("/zlisteners.txt");
    delay(500);
    File f = SPIFFS.open("/zlisteners.txt", "w");
    int ct = 0;
    WiFiServerNode *serv = servs;
    while (serv != nullptr)
    {
        f.printf("%d,%d\n", serv->port, serv->flagsBitmap);
        if (serv->maskOuts == NULL)
            f.printf("0,\n");
        else
            f.printf("%d,%s\n", strlen(serv->maskOuts), serv->maskOuts);
        if (serv->delimiters == NULL)
            f.printf("0,\n");
        else
            f.printf("%d,%s\n", strlen(serv->delimiters), serv->delimiters);
        if (serv->stateMachine == NULL)
            f.printf("0,\n");
        else
            f.printf("%d,%s\n", strlen(serv->stateMachine), serv->stateMachine);
        ct++;
        serv = serv->next;
    }
    f.close();
    delay(500);
    if (SPIFFS.exists("/zlisteners.txt"))
    {
        File f = SPIFFS.open("/zlisteners.txt", "r");
        bool fail = false;
        while (f.available() > 5)
        {
            WiFiServerSpec snode;
            if (!ReadWiFiServer(f, snode))
            {
                fail = true;
                break;
            }
        }
        f.close();
        if (fail)
        {
            delay(100);
            SaveWiFiServers();
        }
    }
}

void WiFiServerNode::DestroyAllServers()
{
    while (servs != nullptr)
    {
        WiFiServerNode *s = servs;
        delete s;
    }
}

WiFiServerNode *WiFiServerNode::FindServer(int port)
{
    WiFiServerNode *s = servs;
    while (s != nullptr)
    {
        if (s->port == port)
            return s;
        s = s->next;
    }
    return nullptr;
}

void WiFiServerNode::RestoreWiFiServers()
{
    if (SPIFFS.exists("/zlisteners.txt"))
    {
        File f = SPIFFS.open("/zlisteners.txt", "r");
        bool fail = false;
        while (f.available() > 0)
        {
            WiFiServerSpec snode;
            if (!ReadWiFiServer(f, snode))
            {
                SerialDebug.println("Server: FAIL");
                fail = true;
                break;
            }
            WiFiServerNode *s = servs;
            while (s != nullptr)
            {
                if (s->port == snode.port)
                    break;
                s = s->next;
            }
            if (s == nullptr)
            {
                WiFiServerNode *node = new WiFiServerNode(snode.port, snode.flagsBitmap);
                setCharArray(&node->delimiters, snode.delimiters);
                setCharArray(&node->maskOuts, snode.maskOuts);
                setCharArray(&node->stateMachine, snode.stateMachine);
                SerialDebug.printf("Server: %d, %d: '%s' '%s'\n", node->port, node->flagsBitmap, node->delimiters, node->maskOuts);
            }
            else
                SerialDebug.printf("Server: DUP %d\n", snode.port);
        }
        f.close();
    }
}

WiFiServerNode *servs = nullptr;