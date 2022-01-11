#ifndef ZSHELL_H
#define ZSHELL_H

#include <Arduino.h>

#define ZSHELL_SHOW_PROMPT  0x01
#define ZSHELL_DONE         0x02

class ZShell
{
public:
    ZShell();
    virtual ~ZShell();

	void begin();
    void end();
    void exec(String line);
    bool done();
private:
    String path;
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