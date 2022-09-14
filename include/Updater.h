#ifndef UPDATER_H
#define UPDATER_H

#include <SPIFFS.h>
#include <StreamString.h>
#include <Update.h>
#include <WebServer.h>

class Updater
{
public:
	Updater();
	virtual ~Updater();

	void setup(WebServer *server);
	void setup(WebServer *server, const String &path);
	void setup(WebServer *server, const String &username, const String &password);
	void setup(WebServer *server, const String &path, const String &username, const String &password);
	void updateCredentials(const String &username, const String &password);

protected:
	void setUpdaterError();

private:
	static const char serverIndex[];
	static const char successResponse[];

	WebServer *m_server;
	String m_username;
	String m_password;
	bool m_authenticated;
	String m_updaterError;
};

#endif