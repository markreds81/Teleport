#ifndef ZPHONEBOOK_H
#define ZPHONEBOOK_H

#include <LinkedList.h>
#include <stdint.h>

#define PHONEBOOK_FILE_NAME "/phonebook.dat"

struct PBEntry
{
    unsigned long number;
    char address[50];
    char modifiers[15];
    char notes[128];

    void update(unsigned long number, const char *address, const char *modifiers, const char *notes);

    static PBEntry *create(unsigned long number, const char *address, const char *modifiers, const char *notes);
};

class ZPhonebook: public LinkedList<struct PBEntry *>
{
public:
    static bool checkEntry(char *cmd);

    ZPhonebook();
    ~ZPhonebook();

    void load();
    void save();

    int findByNumber(unsigned long number);
};

#endif