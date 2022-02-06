#ifndef ZTYPES_H
#define ZTYPES_H

#define ASCII_BS 8
#define ASCII_XON 17
#define ASCII_XOFF 19
#define ASCII_DC4 20
#define ASCII_DELETE 127

#define ENC_TYPE_NONE WIFI_AUTH_OPEN

#include "z/options.h"
#include <stdint.h>

enum FlowControlMode
{
	FCM_DISABLED = 0,
	FCM_UNUSED1 = 1,
	FCM_UNUSED2 = 2,
	FCM_HARDWARE = 3,		// RTS/CTS
	FCM_SOFTWARE = 4,		// XON/XOFF
	FCM_TRANSPARENT = 5,	// transparent XON/XOFF
	FCM_BOTH = 6,			// both RTS/CTS and XON/XOFF
	FCM_INVALID = 7
};

enum ZResult
{
	ZOK = 0,
	ZCONNECT,
	ZRING,
	ZNOCARRIER,
	ZERROR,
	ZNODIALTONE,
	ZBUSY,
	ZNOANSWER,
	ZIGNORE,
	ZIGNORE_SPECIAL
};

enum ZMode
{
	ZCOMMAND_MODE,
	ZCONSOLE_MODE,
	ZSTREAM_MODE,
	ZPRINT_MODE,
	ZSHELL_MODE
};

struct ZEscape {
	unsigned long gt1;
	unsigned long gt2;
	uint8_t buf[ESCAPE_BUF_LEN];
	uint8_t len;
};

#endif