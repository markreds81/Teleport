#include "ZUpdater.h"
#include "ZDebug.h"

const char ZUpdater::serverIndex[] PROGMEM =
	R"(<!DOCTYPE html>
     <html lang='en'>
     <head>
         <meta charset='utf-8'>
         <meta name='viewport' content='width=device-width,initial-scale=1'/>
     </head>
     <body>
     <form method='POST' action='' enctype='multipart/form-data'>
         Firmware:<br>
         <input type='file' accept='.bin,.bin.gz' name='firmware'>
         <input type='submit' value='Update Firmware'>
     </form>
     <form method='POST' action='' enctype='multipart/form-data'>
         FileSystem:<br>
         <input type='file' accept='.bin,.bin.gz,.image' name='filesystem'>
         <input type='submit' value='Update FileSystem'>
     </form>
     </body>
     </html>)";

const char ZUpdater::successResponse[] PROGMEM =
	"<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...";

ZUpdater::ZUpdater()
{
	m_server = NULL;
	m_username = emptyString;
	m_password = emptyString;
	m_authenticated = false;
}

ZUpdater::~ZUpdater()
{
}

void ZUpdater::setup(WebServer *server)
{
	setup(server, emptyString, emptyString);
}

void ZUpdater::setup(WebServer *server, const String &path)
{
	setup(server, path, emptyString, emptyString);
}

void ZUpdater::setup(WebServer *server, const String &username, const String &password)
{
	setup(server, "/update", username, password);
}

void ZUpdater::setup(WebServer *server, const String &path, const String &username, const String &password)
{

	m_server = server;
	m_username = username;
	m_password = password;

	// handler for the /update form page
	m_server->on(path.c_str(), HTTP_GET, [&]()
				{
            if (m_username != emptyString && m_password != emptyString && !m_server->authenticate(m_username.c_str(), m_password.c_str()))
                return m_server->requestAuthentication();
            m_server->send_P(200, PSTR("text/html"), serverIndex); });

	// handler for the /update form POST (once file upload finishes)
	m_server->on(
		path.c_str(), HTTP_POST, [&]()
		{
            if (!m_authenticated)
                return m_server->requestAuthentication();
            if (Update.hasError()) {
                m_server->send(200, F("text/html"), String(F("Update error: ")) + m_updaterError);
            }
            else {
                m_server->client().setNoDelay(true);
                m_server->send_P(200, PSTR("text/html"), successResponse);
                delay(100);
                m_server->client().stop();
                ESP.restart();
            } },
		[&]()
		{
			// handler for the file upload, get's the sketch bytes, and writes
			// them through the Update object
			HTTPUpload &upload = m_server->upload();

			if (upload.status == UPLOAD_FILE_START)
			{
				m_updaterError.clear();

				m_authenticated = (m_username == emptyString || m_password == emptyString || m_server->authenticate(m_username.c_str(), m_password.c_str()));
				if (!m_authenticated)
				{
					DPRINTLN("Unauthenticated Update");
					return;
				}

				DPRINTF("Update: %s\n", upload.filename.c_str());
				if (upload.name == "filesystem")
				{
					if (!Update.begin(SPIFFS.totalBytes(), U_SPIFFS))
					{ // start with max available size
						Update.printError(Serial);
					}
				}
				else
				{
					uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
					if (!Update.begin(maxSketchSpace, U_FLASH))
					{ // start with max available size
						setUpdaterError();
					}
				}
			}
			else if (m_authenticated && upload.status == UPLOAD_FILE_WRITE && !m_updaterError.length())
			{
				DPRINT(".");
				if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
				{
					setUpdaterError();
				}
			}
			else if (m_authenticated && upload.status == UPLOAD_FILE_END && !m_updaterError.length())
			{
				if (Update.end(true))
				{ // true to set the size to the current progress
					DPRINTF("Update Success: %u\nRebooting...\n", upload.totalSize);
				}
				else
				{
					setUpdaterError();
				}
			}
			else if (m_authenticated && upload.status == UPLOAD_FILE_ABORTED)
			{
				Update.end();
				DPRINTLN("Update was aborted");
			}
			delay(0);
		});
}

void ZUpdater::updateCredentials(const String &username, const String &password)
{
	m_username = username;
	m_password = password;
}

void ZUpdater::setUpdaterError()
{
	Update.printError(Serial);
	StreamString str;
	Update.printError(str);
	m_updaterError = str.c_str();
}