// TestShMem.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <iostream>

#define MEM_SIZE 5

int main()
{
    HANDLE hMapFile;
    void* pBuf;

    std::wstring szName = _T("memory_name");

    hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,   // read/write access
        FALSE,                 // do not inherit the name
        szName.c_str());               // name of mapping object

    if (hMapFile == NULL)
    {
        _tprintf(TEXT("Could not open file mapping object (%d).\n"),
            GetLastError());
        return 1;
    }

    while (true)
    {
        pBuf = (LPTSTR)MapViewOfFile(hMapFile, // handle to map object
            FILE_MAP_ALL_ACCESS,  // read/write permission
            0,
            0,
            MEM_SIZE * sizeof(int32_t) + sizeof(int32_t));

        if (pBuf == NULL)
        {
            _tprintf(TEXT("Could not map view of file (%d).\n"),
                GetLastError());

            CloseHandle(hMapFile);

            return 1;
        }

        int32_t* memView = (int32_t*)pBuf;
        int32_t* pBufFlag = memView + MEM_SIZE;

        if (*pBufFlag == 1)
        {
            UnmapViewOfFile(pBuf);
            continue;
        }

        *pBufFlag = 1; //set semaphore
        memView[0] = rand() % 6932; //put random value in the first place of "var" array
        *pBufFlag = 0; //unset semaphore

        UnmapViewOfFile(pBuf);
    }

    UnmapViewOfFile(pBuf);

    CloseHandle(hMapFile);

    return 0;
}
