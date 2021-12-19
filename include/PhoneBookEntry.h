#ifndef PHONE_BOOK_ENTRY_H
#define PHONE_BOOK_ENTRY_H

#include "main.h"

class PhoneBookEntry
{
public:
	unsigned long number;
	const char *address;
	const char *modifiers;
	const char *notes;
	PhoneBookEntry *next = nullptr;

	PhoneBookEntry(unsigned long phnum, const char *addr, const char *mod, const char *note);
	~PhoneBookEntry();

	static void loadPhonebook();
	static void clearPhonebook();
	static void savePhonebook();
	static bool checkPhonebookEntry(String cmd);
	static PhoneBookEntry *findPhonebookEntry(long number);
	static PhoneBookEntry *findPhonebookEntry(String number);
};

extern PhoneBookEntry *phonebook;

#endif