#include "main.h"

#include <sdk/plugin.h>

#include <set>
#include <string>
#include <map>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <cstring>
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
#if defined(WIN32)
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
#else
		key_t shmemKey;

		shmemKey = *(int*)memName;

		int id = shmget(shmemKey, memSize + sizeof(cell), IPC_CREAT | 0666);// + sizeof(cell) because of sync flag

		if (id < 0)
		{
			logprintf("*** Error creating shared memory \"%s\"", memName);
			return 1;
		}

		openMemSegments[memName] = { id, memSize };
#endif
		return 0;
	}

	return 1;
}

static cell AMX_NATIVE_CALL n_DestroyShMemory(AMX* amx, cell* params)
{
	CHECK_PARAMS(1, "OpenShMemory");
	char* memName = NULL;
	amx_StrParam(amx, params[1], memName);

	if (memName != NULL)
	{
		SharedMem* mem = &openMemSegments[memName];

		if (mem != NULL)
		{

#ifdef _WIN32
			CloseHandle(mem->memHandle);
#else
			shmctl(mem->memID, IPC_RMID, NULL);
#endif
			openMemSegments.erase(memName);
			
			return 0;
		}
	}

	return 1;
}

static cell AMX_NATIVE_CALL n_SetShMemoryData(AMX* amx, cell* params)
{
	CHECK_PARAMS(4, "GetShMemoryData");
	//memname, input_buffer, set_size in byte, start_offset in size of elements
	char* memName = NULL;
	amx_StrParam(amx, params[1], memName);
	cell* input = NULL;

	int copySize = static_cast<int>(params[3] * 4); //conversion in byte
	cell startOffset = params[4];

	if (memName != NULL)
	{
		SharedMem* mem = &openMemSegments[memName];

		if (mem != NULL)
		{
#ifdef _WIN32
			void* pBuf = (LPTSTR)MapViewOfFile(mem->memHandle,   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				mem->memSize + sizeof(cell));
#else
			void* pBuf = (void*)shmat(mem->memID, NULL, 0);
#endif

#ifdef _WIN32
			if (pBuf == NULL)
#else
			if (pBuf == (void*)-1)
#endif
			{
#ifdef _WIN32
				logprintf("*** Error viewing shared memory \"%s\" CLOSING..., code: %d ", memName, GetLastError());
				CloseHandle(pBuf);
#else
				logprintf("*** Error viewing shared memory \"%s\" CLOSING...", memName);
				shmctl(mem->memID, IPC_RMID, NULL);
#endif
				openMemSegments.erase(memName);

				return 1;
			}

			cell* flagBuf = (cell*)pBuf + mem->memSize;

			if (*flagBuf == 1)
			{
#ifdef _WIN32
				UnmapViewOfFile(pBuf);
#else
				shmdt(pBuf);
#endif
				//flag is true, it means that another process is accessing it

				return 2;
			}

			//put it on true 
			*flagBuf = 1;

			amx_GetAddr(amx, params[2], &input);

			//overwrite...
			memcpy(pBuf, input + startOffset, copySize);

			*flagBuf = 0;

#ifdef _WIN32
			UnmapViewOfFile(pBuf);
#else
			shmdt(pBuf);
#endif
			return 0;
		}
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
		SharedMem* mem = &openMemSegments[memName];

		if (mem != NULL)
		{
#ifdef _WIN32
			void* pBuf = (LPTSTR)MapViewOfFile(mem->memHandle,   // handle to map object
				FILE_MAP_ALL_ACCESS, // read/write permission
				0,
				0,
				mem->memSize + sizeof(cell));
#else
			void* pBuf = (void*)shmat(mem->memID, NULL, 0);
#endif

#ifdef _WIN32
			if (pBuf == NULL)
#else
			if (pBuf == (void*)-1)
#endif
			{
#ifdef _WIN32
				logprintf("*** Error viewing shared memory \"%s\" CLOSING..., code: %d ", memName, GetLastError());
				CloseHandle(pBuf);
#else
				logprintf("*** Error viewing shared memory \"%s\" CLOSING...", memName);
				shmctl(mem->memID, IPC_RMID, NULL);
#endif
				openMemSegments.erase(memName);

				return 1;
			}

			cell* flagBuf = (cell*)pBuf + mem->memSize;

			if (*flagBuf == 1)
			{
#ifdef _WIN32
				UnmapViewOfFile(pBuf);
#else
				shmdt(pBuf);
#endif
				//flag is true, it means that another process is accessing it

				return 2;
			}

			//put it on true 
			*flagBuf = 1;

			amx_GetAddr(amx, params[2], &dest);

			//overwrite...
			memcpy(dest, pBuf, mem->memSize);

			*flagBuf = 0;

#ifdef _WIN32
			UnmapViewOfFile(pBuf);
#else
			shmdt(pBuf);
#endif
		}
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
		SharedMem* mem = &openMemSegments[memName];

		if (mem != NULL)
		{
			return mem->memSize;
		}
		else
			return -1;
	}

	return -1;
}

AMX_NATIVE_INFO natives[] =
{
	{ "OpenShMemory", n_OpenShMemory },
	{ "DestroyShMemory", n_DestroyShMemory },
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
