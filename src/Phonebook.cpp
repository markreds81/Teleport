#include "Phonebook.h"
#include "DebugPort.h"
#include "string.h"
#include <SPIFFS.h>

PhonebookClass Phonebook;

bool PhonebookClass::checkEntry(char *cmd)
{
    bool error = false;
    for (char *cptr = cmd; *cptr != 0; cptr++)
    {
        if (strchr("0123456789", *cptr) < 0)
        {
            error = true;
        }
    }
    return (error || strlen(cmd) > 9) ? false : true;
}

PhonebookClass::PhonebookClass()
{
}

PhonebookClass::~PhonebookClass()
{
}

int PhonebookClass::indexOf(unsigned long number)
{
    for (int i = 0; i < toc.size(); i++)
    {
        if (toc.get(i) == number)
        {
            return i;
        }
    }
    return -1;
}

void PhonebookClass::begin()
{
    toc.clear();
    File root = SPIFFS.open(ZPHONEBOOK_PREFIX);
    if (root)
    {
        int i = 0;
        File file = root.openNextFile();
        size_t off = strlen(ZPHONEBOOK_PREFIX) + 1;        
        while (file)
        {
            char name[32];
            strcpy(name, file.name() + off);
            char *dot = strchr(name, '.');
            if (dot != NULL)
            {
                *dot = '\0';
                unsigned long number = atol(name);
                toc.add(number);
                DPRINTF("Phonebook entry #%d (%lu) %s\n", i++, number, "found");
            }
            file.close();            
            file = root.openNextFile();
        }
        root.close();
    }
}

bool PhonebookClass::get(int index, PBEntry *pbe)
{
    char name[32];
    size_t bytesRead = 0;
    memset(pbe, 0, sizeof(PBEntry));
    snprintf(name, sizeof(name), "%s/%lu.dat", ZPHONEBOOK_PREFIX, toc.get(index));
    File file = SPIFFS.open(name, "r");
    if (file)
    {
        if (file.available() >= sizeof(PBEntry))
        {
            bytesRead += file.readBytes((char *)pbe, sizeof(PBEntry));
            DPRINTF("Phonebook entry #%d (%lu) %s\n", index, pbe->number, "read");
        }
        file.close();
    }
    return bytesRead > 0;
}

bool PhonebookClass::put(PBEntry *pbe)
{
    char name[32];
    size_t bytesWritten = 0;
    snprintf(name, sizeof(name), "%s/%lu.dat", ZPHONEBOOK_PREFIX, pbe->number);
    File file = SPIFFS.open(name, "w");
    if (file)
    {
        bytesWritten += file.write((uint8_t *)pbe, sizeof(PBEntry));
        file.close();
        int i = indexOf(pbe->number);
        if (i < 0)
        {
            toc.add(pbe->number);
            DPRINTF("Phonebook entry #%d (%lu) %s\n", toc.size() - 1, pbe->number, "added");
        }
        else
        {
            DPRINTF("Phonebook entry #%d (%lu) %s\n", i, pbe->number, "updated");
        }
    }
    return bytesWritten > 0;
}

bool PhonebookClass::put(unsigned long number, const char *address, const char *modifiers, const char *notes)
{
    PBEntry pbe;
    memset(&pbe, 0, sizeof(pbe));
    pbe.number = number;
    if (address != NULL)
        strncpy(pbe.address, address, sizeof(pbe.address));
    if (modifiers != NULL)
        strncpy(pbe.modifiers, modifiers, sizeof(pbe.modifiers));
    if (notes != NULL)
        strncpy(pbe.notes, notes, sizeof(pbe.notes));
    return put(&pbe);
}

void PhonebookClass::remove(int index)
{
    char name[32];
	unsigned long number = toc.get(index);
    snprintf(name, sizeof(name), "%s/%lu.dat", ZPHONEBOOK_PREFIX, number);
    if (SPIFFS.remove(name))
    {
        toc.remove(index);
		DPRINTF("Phonebook entry #%d (%lu) %s\n", index, number, "removed");
    }
}