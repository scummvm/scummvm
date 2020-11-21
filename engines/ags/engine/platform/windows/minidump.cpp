//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS && !AGS_PLATFORM_DEBUG
#define UNICODE
#include <stdio.h> // sprintf
#include "windows.h"
#include <crtdbg.h>
#include "main/main.h"

CONTEXT cpustate;
EXCEPTION_RECORD excinfo;
int miniDumpResultCode = 0;

typedef enum _MINIDUMP_TYPE {
    MiniDumpNormal                         = 0x0000,
    MiniDumpWithDataSegs                   = 0x0001,
    MiniDumpWithFullMemory                 = 0x0002,
    MiniDumpWithHandleData                 = 0x0004,
    MiniDumpFilterMemory                   = 0x0008,
    MiniDumpScanMemory                     = 0x0010,
    MiniDumpWithUnloadedModules            = 0x0020,
    MiniDumpWithIndirectlyReferencedMemory = 0x0040,
    MiniDumpFilterModulePaths              = 0x0080,
    MiniDumpWithProcessThreadData          = 0x0100,
    MiniDumpWithPrivateReadWriteMemory     = 0x0200,
    MiniDumpWithoutOptionalData            = 0x0400,
} MINIDUMP_TYPE;

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef BOOL (WINAPI * MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, 
                                          HANDLE hFile, MINIDUMP_TYPE DumpType, 
                                          CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
                                          CONST void* UserStreamParam, 
                                          CONST void* CallbackParam); 

MINIDUMPWRITEDUMP _MiniDumpWriteDump;


void CreateMiniDump( EXCEPTION_POINTERS* pep ) 
{
    HMODULE dllHandle = LoadLibrary(L"dbghelp.dll");
    if (dllHandle == NULL)
    {
        miniDumpResultCode = 1;
        return;
    }

    _MiniDumpWriteDump = (MINIDUMPWRITEDUMP)GetProcAddress(dllHandle, "MiniDumpWriteDump");
    if (_MiniDumpWriteDump == NULL)
    {
        FreeLibrary(dllHandle);
        miniDumpResultCode = 2;
        return;
    }

    char fileName[80];
    sprintf(fileName, "CrashInfo.%s.dmp", EngineVersion.LongString.GetCStr());
    HANDLE hFile = CreateFileA(fileName, GENERIC_READ | GENERIC_WRITE, 
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 

    if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei; 

        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = pep;
        mdei.ClientPointers = FALSE;

        MINIDUMP_TYPE mdt = MiniDumpNormal; //MiniDumpWithPrivateReadWriteMemory;

        BOOL rv = _MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
            hFile, mdt, (pep != 0) ? &mdei : 0, NULL, NULL); 

        if (!rv)
            miniDumpResultCode = 4;

        CloseHandle(hFile); 
    }
    else
        miniDumpResultCode = 3;

    FreeLibrary(dllHandle);
}

int CustomExceptionHandler (LPEXCEPTION_POINTERS exinfo) {
    cpustate = exinfo->ContextRecord[0];
    excinfo = exinfo->ExceptionRecord[0];
    CreateMiniDump(exinfo);

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif	// AGS_PLATFORM_OS_WINDOWS && !AGS_PLATFORM_DEBUG
