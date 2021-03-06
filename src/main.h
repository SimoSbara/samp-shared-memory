#ifndef MAIN_H
#define MAIN_H

#define PLUGIN_VERSION "1.0"

#include <sdk/plugin.h>
#ifdef _WIN32
	#include <windows.h>
#endif
#include <string>

#define CHECK_PARAMS(m, n) \
	if (params[0] != (m * 4)) \
	{ \
		logprintf("*** %s: Expecting %d parameter(s), but found %d", n, m, params[0] / 4); \
		return 0; \
	}

struct SharedMem
{
#ifdef _WIN32
	HANDLE memHandle;
#else
	int memID;
#endif
	int memSize;
};

typedef void (*logprintf_t)(const char*, ...);

extern logprintf_t logprintf;
extern void *pAMXFunctions;

#endif
