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

enum FlowControlType
{
	FCT_RTSCTS = 0,
	FCT_NORMAL = 1,
	FCT_AUTOOFF = 2,
	FCT_MANUAL = 3,
	FCT_DISABLED = 4,
	FCT_INVALID = 5
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