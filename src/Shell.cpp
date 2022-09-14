#include "Shell.h"
#include "DebugPort.h"
#include "SerialPort.h"
#include <SD.h>

Shell::Shell()
{
	path = "/";
}

Shell::~Shell()
{
}

void Shell::begin(Profile &profile)
{
	if (!SD.begin())
	{
		DPRINTF("SD Card %s\n", "init fails");
	}
	else if (SD.cardType() == CARD_NONE)
	{
		DPRINTF("SD Card %s\n", "not present");
	}
	else
	{
		DPRINTF("SD Card %s\n", "mounted");
	}

	EOLN[0] = profile.carriageReturn();
	if (profile.resultCodeVerbose())
	{
		EOLN[1] = profile.lineFeed();
		EOLN[2] = '\0';
	}
	else
	{
		EOLN[1] = '\0';
	}
	
	state = SHELL_SHOW_PROMPT;
}

void Shell::end()
{
	SD.end();
}

void Shell::exec(String line)
{
	for (int i = 0; i < line.length(); i++)
	{
		if (line[i] < 32)
			line.remove(i--);
	}
	int sp = line.indexOf(' ');
	String cmd = line;
	if (sp > 0)
	{
		cmd = line.substring(0, sp);
		cmd.trim();
		line = line.substring(sp + 1);
		line.trim();
	}
	else
	{
		line = emptyString;
	}

	Serial2.print(EOLN);

	if (cmd.length())
	{
		state |= SHELL_SHOW_PROMPT;

		if (cmd.equalsIgnoreCase("exit") || cmd.equalsIgnoreCase("quit") || cmd.equalsIgnoreCase("x") || cmd.equalsIgnoreCase("endshell"))
		{
			state = SHELL_DONE;
		}
		else if (cmd.equalsIgnoreCase("ls") || cmd.equalsIgnoreCase("dir") || cmd.equalsIgnoreCase("$") || cmd.equalsIgnoreCase("list"))
		{
			String argLetters = "";
			line = stripArgs(line, argLetters);
			argLetters.toLowerCase();
			bool recurse = argLetters.indexOf('r') >= 0;
			String rawPath = makePath(cleanOneArg(line));
			String p;
			String mask;
			if ((line.length() == 0) || (line.endsWith("/")))
			{
				p = rawPath;
				mask = "";
			}
			else
			{
				mask = stripFilename(rawPath);
				if ((mask.length() > 0) && (isMask(mask)))
					p = stripDir(rawPath);
				else
				{
					mask = "";
					p = rawPath;
				}
			}
			showDirectory(p, mask, "", recurse);
		}
		else if (cmd.equalsIgnoreCase("md") || cmd.equalsIgnoreCase("mkdir") || cmd.equalsIgnoreCase("makedir"))
		{
			String p = makePath(cleanOneArg(line));
			DPRINTF("md:%s\n", p.c_str());
			if ((p.length() < 2) || isMask(p) || !SD.mkdir(p))
				Serial2.printf("Illegal path: %s%s", p.c_str(), EOLN);
		}
		else if (cmd.equalsIgnoreCase("cd"))
		{
			String p = makePath(cleanOneArg(line));
			DPRINTF("cd:%s\n", p.c_str());
			if (p.length() == 0)
				Serial2.printf("Current path: %s%s", p.c_str(), EOLN);
			else if (p == "/")
				path = "/";
			else if (p.length() > 1)
			{
				File root = SD.open(p);
				if (!root)
					Serial2.printf("Unknown path: %s%s", p.c_str(), EOLN);
				else if (!root.isDirectory())
					Serial2.printf("Illegal path: %s%s", p.c_str(), EOLN);
				else
					path = p + "/";
			}
		}
		else if (cmd.equalsIgnoreCase("rd") || cmd.equalsIgnoreCase("rmdir") || cmd.equalsIgnoreCase("deletedir"))
		{
			String p = makePath(cleanOneArg(line));
			DPRINTF("rd:%s\n", p.c_str());
			File root = SD.open(p);
			if (!root)
				Serial2.printf("Unknown path: %s%s", p.c_str(), EOLN);
			else if (!root.isDirectory())
				Serial2.printf("Not a directory: %s%s", p.c_str(), EOLN);
			else if (!SD.rmdir(p))
				Serial2.printf("Failed to remove directory: %s%s", p.c_str(), EOLN);
		}
		else if (cmd.equalsIgnoreCase("cat") || cmd.equalsIgnoreCase("type"))
		{
			String p = makePath(cleanOneArg(line));
			DPRINTF("cat:%s\n", p.c_str());
			File root = SD.open(p);
			if (!root)
				Serial2.printf("Unknown path: %s%s", p.c_str(), EOLN);
			else if (root.isDirectory())
				Serial2.printf("Is a directory: %s%s", p.c_str(), EOLN);
			else
			{
				root.close();
				File f = SD.open(p, FILE_READ);
				for (int i = 0; i < f.size(); i++)
					Serial2.write(f.read());
				f.close();
			}
		}
		else if (cmd.equalsIgnoreCase("xget"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("xput"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("zget") || cmd.equalsIgnoreCase("rz") || cmd.equalsIgnoreCase("rz.exe"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("zput") || cmd.equalsIgnoreCase("sz"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("kget") || cmd.equalsIgnoreCase("rk"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("kput") || cmd.equalsIgnoreCase("sk"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("rm") || cmd.equalsIgnoreCase("del") || cmd.equalsIgnoreCase("delete"))
		{
			String argLetters = "";
			line = stripArgs(line, argLetters);
			argLetters.toLowerCase();
			bool recurse = argLetters.indexOf('r') >= 0;
			String rawPath = makePath(cleanOneArg(line));
			String p = stripDir(rawPath);
			String mask = stripFilename(rawPath);
			DPRINTF("rm:%s (%s)\n", p.c_str(), mask.c_str());
			deleteFile(p, mask, recurse);
		}
		else if (cmd.equalsIgnoreCase("cp") || cmd.equalsIgnoreCase("copy"))
		{
			String argLetters = "";
			line = stripArgs(line, argLetters);
			argLetters.toLowerCase();
			bool recurse = argLetters.indexOf('r') >= 0;
			bool overwrite = argLetters.indexOf('f') >= 0;
			String p1 = makePath(cleanFirstArg(line));
			String p2 = makePath(cleanRemainArg(line));
			String mask;
			if ((line.length() == 0) || (line.endsWith("/")))
				mask = "";
			else
			{
				mask = stripFilename(p1);
				if (!isMask(mask))
					mask = "";
				else
					p1 = stripDir(p1);
			}
			DPRINTF("cp:%s (%s) -> %s\n", p1.c_str(), mask.c_str(), p2.c_str());
			copyFiles(p1, mask, p2, recurse, overwrite);
		}
		else if (cmd.equalsIgnoreCase("df") || cmd.equalsIgnoreCase("free") || cmd.equalsIgnoreCase("info"))
		{
			Serial2.printf("%llu free of %llu total%s", (SD.totalBytes() - SD.usedBytes()), SD.totalBytes(), EOLN);
		}
		else if (cmd.equalsIgnoreCase("ren") || cmd.equalsIgnoreCase("rename"))
		{
			String p1 = makePath(cleanFirstArg(line));
			String p2 = makePath(cleanRemainArg(line));
			DPRINTF("ren:%s -> %s\n", p1.c_str(), p2.c_str());
			if (p1 == p2)
				Serial2.printf("File exists: %s%s", p1.c_str(), EOLN);
			else if (SD.exists(p2))
				Serial2.printf("File exists: %s%s", p2.c_str(), EOLN);
			else
			{
				if (!SD.rename(p1, p2))
					Serial2.printf("Failed to rename: %s%s", p1.c_str(), EOLN);
			}
		}
		else if (cmd.equalsIgnoreCase("wget"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("fget"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("fput"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("fls") || cmd.equalsIgnoreCase("fdir"))
		{
			DPRINTLN("Not implemented");
		}
		else if (cmd.equalsIgnoreCase("mv") || cmd.equalsIgnoreCase("move"))
		{
			String argLetters = "";
			line = stripArgs(line, argLetters);
			argLetters.toLowerCase();
			bool overwrite = argLetters.indexOf('f') >= 0;
			String p1 = makePath(cleanFirstArg(line));
			String p2 = makePath(cleanRemainArg(line));
			String mask;
			if ((line.length() == 0) || (line.endsWith("/")))
				mask = "";
			else
			{
				mask = stripFilename(p1);
				if ((mask.length() > 0) && (isMask(mask)))
					p1 = stripDir(p1);
				else
					mask = "";
			}
			DPRINTF("mv:%s(%s) -> %s\n", p1.c_str(), mask.c_str(), p2.c_str());
			if ((mask.length() == 0) || (!isMask(mask)))
			{
				File root = SD.open(p2);
				if (root && root.isDirectory())
				{
					if (!p2.endsWith("/"))
						p2 += "/";
					p2 += stripFilename(p1);
					DPRINTF("mv:%s -> %s\n", p1.c_str(), p2.c_str());
				}
				root.close();
				if (p1 == p2)
					Serial2.printf("File exists: %s%s", p1.c_str(), EOLN);
				else if (SD.exists(p2) && (!overwrite))
					Serial2.printf("File exists: %s%s", p2.c_str(), EOLN);
				else
				{
					if (SD.exists(p2))
						SD.remove(p2);
					if (!SD.rename(p1, p2))
						Serial2.printf("Failed to move: %s%s", p1.c_str(), EOLN);
				}
			}
			else
			{
				copyFiles(p1, mask, p2, false, overwrite);
				deleteFile(p1, mask, false);
			}
		}
		else if (cmd.equals("?") || cmd.equals("help"))
		{
			Serial2.printf("Commands:%s", EOLN);
			Serial2.printf("ls/dir/list/$ [-r] [/][path]                   - List files%s", EOLN);
			Serial2.printf("cd [/][path][..]                               - Change to new directory%s", EOLN);
			Serial2.printf("md/mkdir/makedir [/][path]                     - Create a new directory%s", EOLN);
			Serial2.printf("rd/rmdir/deletedir [/][path]                   - Delete a directory%s", EOLN);
			Serial2.printf("rm/del/delete [-r] [/][path]filename           - Delete a file%s", EOLN);
			Serial2.printf("cp/copy [-r] [-f] [/][path]file [/][path]file  - Copy file(s)%s", EOLN);
			Serial2.printf("ren/rename [/][path]file [/][path]file         - Rename a file%s", EOLN);
			Serial2.printf("mv/move [-f] [/][path]file [/][path]file       - Move file(s)%s", EOLN);
			Serial2.printf("cat/type [/][path]filename                     - View a file(s)%s", EOLN);
			Serial2.printf("df/free/info                                   - Show space remaining%s", EOLN);
			Serial2.printf("xget/zget/kget [/][path]filename               - Download a file%s", EOLN);
			Serial2.printf("xput/zput/kput [/][path]filename               - Upload a file%s", EOLN);
			Serial2.printf("wget [http://url] [/][path]filename            - Download url to file%s", EOLN);
			Serial2.printf("fget [ftp://user:pass@url/file] [/][path]file  - FTP get file%s", EOLN);
			Serial2.printf("fput [/][path]file [ftp://user:pass@url/file]  - FTP put file%s", EOLN);
			Serial2.printf("fdir [ftp://user:pass@url/path]                - ftp url dir%s", EOLN);
			Serial2.printf("exit/quit/x/endshell                           - Quit to command mode%s", EOLN);
			Serial2.printf("%s", EOLN);
		}
		else
		{
			Serial2.printf("Unknown command: '%s'.  Try '?'.%s", cmd.c_str(), EOLN);
		}
	}
}

bool Shell::done()
{
	if (state & SHELL_SHOW_PROMPT)
	{
		state &= ~SHELL_SHOW_PROMPT;
		Serial2.printf("%s%s> ", EOLN, path.c_str());
	}
	return (state & SHELL_DONE) == SHELL_DONE;
}

bool Shell::isMask(String mask)
{
	return (mask.indexOf("*") >= 0) || (mask.indexOf("?") >= 0);
}

bool Shell::matches(String fname, String mask)
{
	if ((mask.length() == 0) || (mask.equals("*")))
		return true;
	if (!isMask(mask))
		return mask.equals(fname);
	int f = 0;
	for (int i = 0; i < mask.length(); i++)
	{
		if (f >= fname.length())
			return false;
		if (mask[i] == '?')
			f++;
		else if (mask[i] == '*')
		{
			if (i == mask.length() - 1)
				return true;
			int remain = mask.length() - i - 1;
			f = fname.length() - remain;
		}
		else if (mask[i] != fname[f++])
			return false;
	}
	return true;
}

String Shell::stripArgs(String line, String &argLetters)
{
	while (line.startsWith("-"))
	{
		int x = line.indexOf(' ');
		if (x < 0)
		{
			argLetters = line.substring(1);
			return emptyString;
		}
		argLetters += line.substring(1, x);
		line = line.substring(x + 1);
		line.trim();
	}
	return line;
}

String Shell::makePath(String addendum)
{
	if (addendum.length() > 0)
	{
		return addendum.startsWith("/") ? fixPathNoSlash(addendum) : fixPathNoSlash(path + addendum);
	}
	return fixPathNoSlash(path);
}

String Shell::cleanOneArg(String line)
{
	int state = 0;
	String arg = "";
	for (int i = 0; i < line.length(); i++)
	{
		if ((line[i] == '\\') && (i < line.length() - 1))
		{
			i++;
			arg += line[i];
		}
		else if (line[i] == '\"')
		{
			if ((state == 0) && (arg.length() == 0))
				state = 1;
			else if (state == 1)
				break;
			else
				arg += line[i];
		}
		else
			arg += line[i];
	}
	return arg;
}

String Shell::fixPathNoSlash(String p)
{
	String finalPath = "";
	int lastX = 0;
	uint16_t backStack[256] = {0};
	backStack[0] = 1;
	int backX = 1;
	for (int i = 0; i < p.length() && i < 512; i++)
	{
		if (p[i] == '/')
		{
			if (i > lastX)
			{
				String sub = p.substring(lastX, i);
				if (sub.equals("."))
				{
					// do nothing
				}
				else if (sub.equals(".."))
				{
					if (backX > 1)
						finalPath = finalPath.substring(0, backStack[--backX]);
				}
				else if (sub.length() > 0)
				{
					finalPath += sub;
					finalPath += "/";
					backStack[++backX] = finalPath.length();
				}
			}
			else if ((i == 0) && (i < p.length() - 1))
				finalPath = "/";
			lastX = i + 1;
		}
	}

	if (lastX < p.length())
	{
		String sub = p.substring(lastX);
		if (sub.equals("."))
		{
			// do nothing
		}
		else if (sub.equals(".."))
		{
			if (backX > 1)
				finalPath = finalPath.substring(0, backStack[--backX]);
		}
		else
		{
			finalPath += sub;
			finalPath += "/"; // why this?! -- oh, so it can be removed below?
		}
	}
	if (finalPath.length() == 0)
		return "/";
	if (finalPath.length() > 1)
		finalPath.remove(finalPath.length() - 1);
	return finalPath;
}

String Shell::stripFilename(String p)
{
	int x = p.lastIndexOf("/");
	if ((x < 0) || (x == p.length() - 1))
		return "";
	return p.substring(x + 1);
}

String Shell::stripDir(String p)
{
	int x = p.lastIndexOf("/");
	if (x <= 0)
		return "/";
	return p.substring(0, x);
}

String Shell::cleanFirstArg(String line)
{
	int state = 0;
	String arg = "";
	for (int i = 0; i < line.length(); i++)
	{
		if ((line[i] == '\\') && (i < line.length() - 1))
		{
			i++;
			arg += line[i];
		}
		else if (line[i] == '\"')
		{
			if ((state == 0) && (arg.length() == 0))
				state = 1;
			else if (state == 1)
				break;
			else
				arg += line[i];
		}
		else if (line[i] == ' ')
		{
			if (state == 0)
				break;
			arg += line[i];
		}
		else
		{
			arg += line[i];
		}
	}
	return arg;
}

String Shell::cleanRemainArg(String line)
{
	int state = 0;
	int ct = 0;
	for (int i = 0; i < line.length(); i++)
	{
		if ((line[i] == '\\') && (i < line.length() - 1))
		{
			i++;
			ct++;
		}
		else if (line[i] == '\"')
		{
			if ((state == 0) && (ct == 0))
				state = 1;
			else if (state == 1)
			{
				String remain = line.substring(i + 1);
				remain.trim();
				return cleanOneArg(remain);
			}
			else
				ct++;
		}
		else if (line[i] == ' ')
		{
			if (state == 0)
			{
				String remain = line.substring(i + 1);
				remain.trim();
				return cleanOneArg(remain);
			}
			ct++;
		}
		else
			ct++;
	}
	return "";
}

void Shell::showDirectory(String p, String mask, String prefix, bool recurse)
{
	int maskFilterLen = p.length();
	if (!p.endsWith("/"))
		maskFilterLen++;

	File root = SD.open(p);
	if (!root)
		Serial2.printf("Unknown path: %s%s\n", p.c_str(), EOLN);
	else if (root.isDirectory())
	{
		File file = root.openNextFile();
		while (file)
		{
			if (matches(file.name() + maskFilterLen, mask))
			{
				DPRINTF("file matched:%s\n", file.name());
				if (file.isDirectory())
				{
					Serial2.printf("%sd %s%s", prefix.c_str(), file.name() + maskFilterLen, EOLN);
					if (recurse)
					{
						String newPrefix = prefix + "  ";
						showDirectory(file.name(), mask, newPrefix, recurse);
					}
				}
				else
					Serial2.printf("%s  %s %d%s", prefix.c_str(), file.name() + maskFilterLen, file.size(), EOLN);
			}
			else
				DPRINTF("file unmatched:%s (%s)\n", file.name(), mask.c_str());
			file = root.openNextFile();
		}
	}
	else
		Serial2.printf("  %s %d%s", root.name(), root.size(), EOLN);
}

void Shell::deleteFile(String p, String mask, bool recurse)
{
	int maskFilterLen = p.length();
	if (!p.endsWith("/"))
		maskFilterLen++;

	File root = SD.open(p);
	if (!root)
		Serial2.printf("Unknown path: %s%s", p.c_str(), EOLN);
	else if (root.isDirectory())
	{
		File file = root.openNextFile();
		while (file)
		{
			if (matches(file.name() + maskFilterLen, mask))
			{
				String fileName = file.name();
				if (file.isDirectory())
				{
					if (recurse)
					{
						file = root.openNextFile();
						deleteFile(fileName.c_str(), "*", recurse);
						if (!SD.rmdir(fileName.c_str()))
							Serial2.printf("Unable to delete: %s%s", fileName.c_str() + maskFilterLen, EOLN);
					}
					else
					{
						Serial2.printf("Skipping: %s%s", file.name() + maskFilterLen, EOLN);
						file = root.openNextFile();
					}
				}
				else
				{
					file = root.openNextFile();
					if (!SD.remove(fileName))
						Serial2.printf("Unable to delete: %s%s", file.name() + maskFilterLen, EOLN);
				}
			}
			else
				file = root.openNextFile();
		}
	}
}

void Shell::copyFiles(String source, String mask, String target, bool recurse, bool overwrite)
{
	int maskFilterLen = source.length();
	if (!source.endsWith("/"))
		maskFilterLen++;

	File root = SD.open(source);
	if (!root)
	{
		Serial2.printf("Unknown path: %s%s", source.c_str(), EOLN);
		return;
	}

	if (root.isDirectory())
	{
		if (!SD.exists(target)) // cp d a
		{
			SD.mkdir(target);
		}
		else
		{
			File DD = SD.open(target); // cp d d2, cp d f
			if (!DD.isDirectory())
			{
				Serial2.printf("File exists: %s%s", DD.name(), EOLN);
				DD.close();
				return;
			}
		}
		for (File file = root.openNextFile(); !file; file = root.openNextFile())
		{
			if (matches(file.name() + maskFilterLen, mask))
			{
				DPRINTF("file matched:%s\n", file.name());
				String tpath = target;
				if (file.isDirectory())
				{
					if (!recurse)
						Serial2.printf("Skipping: %s%s", file.name(), EOLN);
					else
					{
						if (!tpath.endsWith("/"))
							tpath += "/";
						tpath += stripFilename(file.name());
					}
				}
				copyFiles(file.name(), "", tpath, false, overwrite);
			}
		}
	}
	else
	{
		String tpath = target;
		if (SD.exists(tpath))
		{
			File DD = SD.open(tpath);
			if (DD.isDirectory()) // cp f d, cp f .
			{
				if (!tpath.endsWith("/"))
					tpath += "/";
				tpath += stripFilename(root.name());
				DPRINTF("file xform to file in dir:%s\n", tpath.c_str());
			}
			DD.close();
		}
		if (SD.exists(tpath))
		{
			File DD = SD.open(tpath);
			if (strcmp(DD.name(), root.name()) == 0)
			{
				Serial2.printf("File exists: %s%s", DD.name(), EOLN);
				DD.close();
				return;
			}
			else if (!overwrite) // cp f a, cp f e
			{
				Serial2.printf("File exists: %s%s", DD.name(), EOLN);
				DD.close();
				return;
			}
			else
			{
				DD.close();
				SD.remove(tpath);
			}
		}
		size_t len = root.size();
		File tfile = SD.open(tpath, FILE_WRITE);
		for (int i = 0; i < len; i++)
		{
			uint8_t c = root.read();
			tfile.write(c);
		}
		tfile.close();
	}
}