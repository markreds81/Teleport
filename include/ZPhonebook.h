#ifndef ZPHONEBOOK_H
#define ZPHONEBOOK_H

#include <LinkedList.h>
#include <stdint.h>

struct PBEntry
{
    unsigned long number;
    char hostname[32];
    uint16_t port;
    char modifiers[15];
    char notes[128];
};

class ZPhonebook: public LinkedList<struct PBEntry *>
{
public:
    ZPhonebook();
    ~ZPhonebook();

    void load();
    void save();

    PBEntry *findByNamber(unsigned long number);
};

#endif