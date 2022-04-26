#ifndef MAIN_H
#define MAIN_H

#define PLUGIN_VERSION "1.0"

#include <sdk/plugin.h>
#include <windows.h>
#include <string>
#include <vector>
#include <atomic>

#define CHECK_PARAMS(m, n) \
	if (params[0] != (m * 4)) \
	{ \
		logprintf("*** %s: Expecting %d parameter(s), but found %d", n, m, params[0] / 4); \
		return 0; \
	}

struct SharedMem
{
#ifdef LINUX
	int memID;
#elif defined(WIN32)
	HANDLE memHandle;
#endif
	int memSize;
};

typedef void (*logprintf_t)(const char*, ...);

extern logprintf_t logprintf;
extern void *pAMXFunctions;

#endif
