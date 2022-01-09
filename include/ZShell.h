#ifndef ZSHELL_H
#define ZSHELL_H

#include <Arduino.h>

class ZShell
{
public:
    ZShell();
    virtual ~ZShell();

	bool exec(String line);
	void tick();
};

#endif