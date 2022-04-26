#include "main.h"

#include <sdk/plugin.h>

#include <queue>
#include <set>
#include <string>
#include <vector>
#include <map>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

std::set<AMX*> interfaces;

logprintf_t logprintf;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	logprintf("\n\nShared Memory Plugin by SimoSbara loaded\n", PLUGIN_VERSION);
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	logprintf("\n\nShared Memory Plugin by SimoSbara unloaded\n", PLUGIN_VERSION);    
}

static std::map<std::string, SharedMem> openMemSegments;

static cell AMX_NATIVE_CALL n_OpenShMemory(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "OpenShMemory");
	char* memName = NULL;
	amx_StrParam(amx, params[1], memName);
	int memSize = static_cast<int>(params[2]);

	//sizeof of pawn means number of elements, we need to convert it in byte
	memSize = memSize * sizeof(cell);

	if (memName != NULL)
	{
#ifdef LINUX
		int id = shmget(key, memSize + 1, IPC_CREAT | 0666);// +1 because of the flag of sync semaphore  

		if (id < 0)
		{
			logprintf("*** Error creating shared memory \"%s\"", memName);
			return 1;
		}

		openMemSegments.insert(memName, { id, newMem });
#elif defined(WIN32)
		HANDLE mappedMemory = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			memSize + sizeof(cell),                // maximum object size (low-order DWORD) // +sizeof cell because of the flag of sync semaphore  
			memName); // name of mapping object

		if (mappedMemory == NULL)
		{
			logprintf("*** Error creating shared memory \"%s\", code: %d ", memName, GetLastError());
			return 1;
		}

		void* pBuf = (LPTSTR)MapViewOfFile(mappedMemory,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			memSize + sizeof(cell));

		if (pBuf == NULL)
		{
			logprintf("*** Error viewing shared memory \"%s\" CLOSING..., code: %d ", memName, GetLastError());
			CloseHandle(mappedMemory);

			return 1;
		}

		//flag to false
		cell* flag = (cell*)pBuf + memSize;

		*flag = 0;

		UnmapViewOfFile(pBuf);

		openMemSegments[memName] = { mappedMemory, memSize };
#endif
		return 0;
	}

	return 1;
}

static cell AMX_NATIVE_CALL n_SetShMemoryData(AMX* amx, cell* params)
{
	CHECK_PARAMS(4, "GetShMemoryData");
	//memname, input_buffer, set_size in byte, start_offset in byte
	char* memName = NULL;
	amx_StrParam(amx, params[1], memName);
	cell* input = NULL;

	int copySize = static_cast<int>(params[3] * 4);
	cell startOffset = params[4];

	if (memName != NULL)
	{
#ifdef LINUX

#elif defined(WIN32)

		SharedMem* mem = &openMemSegments[memName];
		
		if (mem != NULL)
		{
			void* pBuf = (LPTSTR)MapViewOfFile(mem->memHandle,   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				mem->memSize + sizeof(cell));

			if (pBuf == NULL)
			{
				logprintf("*** Error viewing shared memory \"%s\" CLOSING..., code: %d ", memName, GetLastError());
				CloseHandle(mem->memHandle);

				openMemSegments.erase(memName);

				return 1;
			}

			cell* flagBuf = (cell*)pBuf + mem->memSize;

			if (*flagBuf == 1)
			{
				UnmapViewOfFile(pBuf);

				//flag is true, it means that another process is accessing it

				return 2;
			}

			//put it on true 
			*flagBuf = 1;

			amx_GetAddr(amx, params[2], &input);

			//overwrite...
			memcpy(pBuf, input + startOffset, copySize);

			*flagBuf = 0;

			UnmapViewOfFile(pBuf);

			return 0;
		}
#endif
	}

	return 1;
}

static cell AMX_NATIVE_CALL n_GetShMemoryData(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "GetShMemoryData");
	char* memName = NULL;
	amx_StrParam(amx, params[1], memName);
	cell* dest = NULL;

	if (memName != NULL)
	{
#ifdef LINUX

#elif defined(WIN32)
		SharedMem* mem = &openMemSegments[memName];

		if (mem != NULL)
		{
			void* pBuf = (LPTSTR)MapViewOfFile(mem->memHandle,   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				mem->memSize + sizeof(cell));

			if (pBuf == NULL)
			{
				logprintf("*** Error viewing shared memory \"%s\" CLOSING..., code: %d ", memName, GetLastError());
				CloseHandle(mem->memHandle);

				openMemSegments.erase(memName);

				return 1;
			}

			cell* flagBuf = (cell*)pBuf + mem->memSize;

			if (*flagBuf == 1)
			{
				UnmapViewOfFile(pBuf);

				//flag is true, it means that another process is accessing it

				return 2;
			}

			//put it on true 
			*flagBuf = 1;

			amx_GetAddr(amx, params[2], &dest);

			//overwrite...
			memcpy(dest, pBuf, mem->memSize);

			*flagBuf = 0;

			UnmapViewOfFile(pBuf);

			return 0;
		}
#endif
	}

	return 1;
}

static cell AMX_NATIVE_CALL n_GetShMemorySize(AMX* amx, cell* params)
{
	CHECK_PARAMS(1, "GetShMemorySize");
	char* memName = NULL;
	amx_StrParam(amx, params[1], memName);

	if (memName != NULL)
	{
#ifdef LINUX

#elif defined(WIN32)
		SharedMem* mem = &openMemSegments[memName];

		if (mem != NULL)
		{
			return mem->memSize;
		}
		else
			return -1;
#endif
	}

	return -1;
}

AMX_NATIVE_INFO natives[] =
{
	{ "OpenShMemory", n_OpenShMemory },
	{ "GetShMemoryData", n_GetShMemoryData },
	{ "SetShMemoryData", n_SetShMemoryData },
	{ "GetShMemorySize", n_GetShMemorySize },
	{ 0, 0 }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	interfaces.insert(amx);
	return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	interfaces.erase(amx);
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	return;
}
