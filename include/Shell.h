#ifndef SHELL_H
#define SHELL_H

#include "Profile.h"
#include <Arduino.h>

#define SHELL_SHOW_PROMPT  0x01
#define SHELL_DONE         0x02

class Shell
{
public:
    Shell();
    virtual ~Shell();

	void begin(Profile &profile);
    void end();
    void exec(String line);
    bool done();
private:
    String path;
    char EOLN[3];
    uint8_t state;

    bool isMask(String mask);
    bool matches(String fname, String mask);
    String stripArgs(String line, String &argLetters);
    String makePath(String addendum);
    String cleanOneArg(String line);
    String fixPathNoSlash(String p);
    String stripFilename(String p);
    String stripDir(String p);
    String cleanFirstArg(String line);
    String cleanRemainArg(String line);
    void showDirectory(String p, String mask, String prefix, bool recurse);
    void deleteFile(String p, String mask, bool recurse);
    void copyFiles(String source, String mask, String target, bool recurse, bool overwrite);
};

#endif