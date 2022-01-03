#include "ZPhonebook.h"
#include "ZDebug.h"
#include "string.h"
#include <SPIFFS.h>

PBEntry *PBEntry::create(unsigned long number, const char *address, const char *modifiers, const char *notes)
{
    PBEntry *pbe = new PBEntry();

    pbe->update(number, address, modifiers, notes);

    return pbe;
}

void PBEntry::update(unsigned long number, const char *address, const char *modifiers, const char *notes)
{
    memset(this, 0, sizeof(PBEntry));
    this->number = number;
    if (address != NULL)
        strncpy(this->address, address, sizeof(this->address));
    if (modifiers != NULL)
        strncpy(this->modifiers, modifiers, sizeof(this->modifiers));
    if (notes != NULL)
        strncpy(this->notes, notes, sizeof(this->notes));
}

bool ZPhonebook::checkEntry(char *cmd)
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

ZPhonebook::ZPhonebook()
{
    // NOP
}

ZPhonebook::~ZPhonebook()
{
}

void ZPhonebook::load()
{
    File file = SPIFFS.open(PHONEBOOK_FILE_NAME, "r");
    if (file)
    {
        size_t bytesRead = 0;
        while (file.available() >= sizeof(PBEntry))
        {
            PBEntry *pbe = new PBEntry();
            bytesRead += file.readBytes((char *)pbe, sizeof(PBEntry));
            this->add(pbe);
        }
        file.close();
        DPRINTF("Phonebook loaded, %d bytes %s\n", bytesRead, "read");
    }
}

void ZPhonebook::save()
{
    File file = SPIFFS.open(PHONEBOOK_FILE_NAME, "w");
    if (file)
    {
        size_t bytesWritten = 0;
        for (int i = 0; i < this->size(); i++)
        {
            PBEntry *pbe = this->get(i);
            bytesWritten += file.write((uint8_t *)pbe, sizeof(PBEntry));
        }
        file.close();
        DPRINTF("Phonebook saved, %d bytes %s\n", bytesWritten, "written");
    }
}

int ZPhonebook::findByNumber(unsigned long number)
{
    int i;
    bool found = false;
    for (i = 0; i < this->size(); i++)
    {
        PBEntry *pbe = this->get(i);
        if (pbe->number == number)
        {
            found = true;
            break;
        }
    }
    return found ? i : -1;
}