// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifdef BS_PLATFORM_WINDOWS

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ShellAPI.h>
#include <stdio.h>

#include <vector>
using namespace std;

#include "kernel/common.h"
#include "kernel/kernel.h"

#define BS_LOG_PREFIX "MAIN_WIN"

// -----------------------------------------------------------------------------

namespace
{
	const char * const ENGINE_STARTUP_ERROR_MESSAGE = "A fatal error occured during engine startup. Please refer to log.txt for further information.";
	const char * const ENGINE_STARTUP_ERROR_CAPTION = "Broken Sword 2.5";
	const char * const EXCEPTION_TERMINATION_MESSAGE = "!! PROGRAM TERMINATED DUE TO EXCEPTION !!";

	// -------------------------------------------------------------------------

	#define XXX(EX) case EX: return #EX;
	const char * GetExceptionCodeString(DWORD ExceptionCode)
	{
		switch (ExceptionCode)
		{
			XXX(EXCEPTION_ACCESS_VIOLATION)
			XXX(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
			XXX(EXCEPTION_BREAKPOINT)
			XXX(EXCEPTION_DATATYPE_MISALIGNMENT)
			XXX(EXCEPTION_FLT_DENORMAL_OPERAND)
			XXX(EXCEPTION_FLT_DIVIDE_BY_ZERO)
			XXX(EXCEPTION_FLT_INEXACT_RESULT)
			XXX(EXCEPTION_FLT_INVALID_OPERATION)
			XXX(EXCEPTION_FLT_OVERFLOW)
			XXX(EXCEPTION_FLT_STACK_CHECK)
			XXX(EXCEPTION_FLT_UNDERFLOW)
			XXX(EXCEPTION_GUARD_PAGE)
			XXX(EXCEPTION_ILLEGAL_INSTRUCTION)
			XXX(EXCEPTION_IN_PAGE_ERROR)
			XXX(EXCEPTION_INT_DIVIDE_BY_ZERO)
			XXX(EXCEPTION_INT_OVERFLOW)
			XXX(EXCEPTION_INVALID_DISPOSITION)
			XXX(EXCEPTION_INVALID_HANDLE)
			XXX(EXCEPTION_NONCONTINUABLE_EXCEPTION)
			XXX(EXCEPTION_PRIV_INSTRUCTION)
			XXX(EXCEPTION_SINGLE_STEP)
			XXX(EXCEPTION_STACK_OVERFLOW)
			XXX(DBG_CONTROL_C)
			XXX(DBG_CONTROL_BREAK)
			XXX(DBG_TERMINATE_THREAD)
			XXX(DBG_TERMINATE_PROCESS)
			XXX(RPC_S_UNKNOWN_IF)
			XXX(RPC_S_SERVER_UNAVAILABLE)
		default:
			static char Buffer[64];
			_snprintf(Buffer, sizeof(Buffer), "UNKNOWN EXCEPTION 0x%08X", ExceptionCode);
			return Buffer;
		}
	}
	#undef XXX

	// -------------------------------------------------------------------------

	int HandleException(unsigned int ExceptionCode, _EXCEPTION_POINTERS * ExceptionPointersPtr)
	{
		BS_LOG_ERRORLN("Exception \"%s\" occured at 0x%08X.", GetExceptionCodeString(ExceptionCode), ExceptionPointersPtr->ContextRecord->Eip);

		// Memorydump schreiben
		std::string Filename;

		return EXCEPTION_EXECUTE_HANDLER;
	}

}

// -----------------------------------------------------------------------------

extern bool AppStart(const vector<string> & CommandLineParameters);
extern bool AppMain();
extern bool AppEnd();

bool main2(int argc, char ** argv)
{
	// Engine initialisieren.
	vector<string> CommandLineParameters;
	for (int i = 0; i < argc; ++i) CommandLineParameters.push_back(string(argv[i]));
	if (!AppStart(CommandLineParameters))
	{
		MessageBoxA(0, ENGINE_STARTUP_ERROR_MESSAGE, ENGINE_STARTUP_ERROR_CAPTION, MB_ICONERROR);
		AppEnd();
		return 1;
	}

	// Engine starten.
	bool RunSuccess = AppMain();

	// Engine deinitialisieren.
	bool DeinitSuccess = AppEnd();

	return (RunSuccess && DeinitSuccess) ? 0 : 1;
}

// -----------------------------------------------------------------------------

int main(int argc, char ** argv)
{
	// Im Release-Modus wird die gesamte Ausführung wird in einen __try-Block eingebettet, damit alle eventuellen Exceptions abgefangen werden.
	// Im Debug-Modus wollen wir, dass der Debugger die Exceptions fängt.
#ifndef DEBUG
	__try
#endif
	{
		// Im Debugmodus erlauben wir mehrere Fenster, ansonsten beenden wir uns wenn das Spiel bereits läuft;
		// Wichtig ist vor allem dass all dies vor Installation des Logservice passiert, da wir sonst das
		// Log der laufenden Instanz überschreiben würden.
#ifndef DEBUG
		HANDLE hMutex;
		hMutex = CreateMutex(NULL, TRUE, "137bd040-8f06-11dd-8299-0016e65b9c32");
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			return 1;
		}
#endif

		// Der weitere Teil ist in eine andere Funktion ausgelagert, da Visual C++ in Funktionen die auch SEH benutzen keine Objekte mit Destruktoren zulässt.
		return main2(argc, argv);
	}

#ifndef DEBUG
	__except(HandleException(GetExceptionCode(), GetExceptionInformation()))
	{
		BS_Kernel::DeleteInstance();
		BS_LOG_ERRORLN(EXCEPTION_TERMINATION_MESSAGE);
		return 1;
	}
#endif
}

// -----------------------------------------------------------------------------

namespace
{
	size_t CalculateConvertedBufferSize(int NumArgs, LPWSTR * Args)
	{
		if (NumArgs > 0 && Args)
		{
			size_t BufferSize = NumArgs * sizeof(char *);
			for (int i = 0; i < NumArgs; ++i)
			{
				int Length = WideCharToMultiByte(CP_ACP, 0, Args[i], -1, 0, 0, 0, 0);
				if (Length == 0) return 0;
				BufferSize += Length;
			}

			return BufferSize;
		}
		else
		{
			return 0;
		}
	}

	// -------------------------------------------------------------------------

	bool ConvertCommandLineParametersToANSI(int & NumArgs, vector<char> & Buffer)
	{
		// Kommandozeilenparameter auslesen.
		LPWSTR * Args = CommandLineToArgvW(GetCommandLineW(), &NumArgs);
		if (NumArgs <= 0 || Args == 0) false;

		// Zuerst berechnen, wie groß der Buffer sein muss, der alle konvertierten Strings und die Stringpointer aufnimmt.
		size_t BufferSize = CalculateConvertedBufferSize(NumArgs, Args);
		if (BufferSize == 0) false;

		// Bufferspeicher reservieren.
		Buffer.resize(BufferSize);

		// Die benötigten Pointer erstellen.
		char ** StringsPtrPtr = reinterpret_cast<char **>(&Buffer[0]);	// Pointer auf den Anfang des Buffers, dort werden die Stringpointer abgelegt.
		char * StringsPtr = &Buffer[NumArgs * sizeof(char *)];			// Pointer auf den Anfang des Bereiches in dem die Strings abgelegt werden.
		char * EndPtr = &Buffer[0] + Buffer.size();						// Pointer auf das Ende des Buffers, wird zum Berechnen des verbleibenden Platzes benötigt.

		// Alle Strings konvertieren und zusammen mit den Pointern in den Buffer schreiben.
		for (int i = 0; i < NumArgs; ++i)
		{
			int BytesWritten = WideCharToMultiByte(CP_ACP, 0, Args[i], -1, StringsPtr, EndPtr - StringsPtr, 0, 0);
			if (BytesWritten == 0) return false;
			StringsPtrPtr[i] = StringsPtr;
			StringsPtr += BytesWritten;
		}

		return true;
	}
}

// -----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	int NumArgs;
	vector<char> Buffer;
	if (ConvertCommandLineParametersToANSI(NumArgs, Buffer))
	{
		main(NumArgs, reinterpret_cast<char **>(&Buffer[0]));
	}
	else
		main(0, 0);
}

#endif // BS_PLATFORM_WINDOWS
