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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "..\filesystemutil.h"
#include "memorydumper.h"
#include "debugtools.h"

using namespace std;

#define BS_LOG_PREFIX "MEMORYDUMPER"

// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

namespace
{
	const char * DBG_HELP_DLL_FILENAME = "dbghelp.dll";
	const char * MINIDUMPWRITEDUMP_FUNCTIONNAME = "MiniDumpWriteDump";
	const char * DUMPS_DIRECTORYNAME = "dumps";
	const char * DUMPFILE_EXTENSION = ".dmp";
}

// -----------------------------------------------------------------------------
// Construction / Destruction
// -----------------------------------------------------------------------------

namespace
{
	HMODULE LoadDbghelpDLL()
	{
		// Zunächst wird versucht die DLL in dem Verzeichnis der EXE-Datei zu finden, da diese die gewünschte Funktionalität unterstützt

		// Pfad der EXE-Datei bestimmen
		char ExePath[MAX_PATH];
		if (GetModuleFileNameA(0, ExePath, sizeof(ExePath)))
		{
			// EXE-Dateinamen abschneiden und den DLL-Dateinamen anhängen
			string DllPath = ExePath;
			string::size_type SlashPos;
			if ((SlashPos = DllPath.rfind("\\")) != string::npos)
			{
				DllPath.resize(SlashPos + 1);
				DllPath += DBG_HELP_DLL_FILENAME;

				HMODULE DllModule = LoadLibraryA(DllPath.c_str());
				if (DllModule) return DllModule;
			}
		}

		// Falls dies fehlgeschlagen ist, wird versucht die Version zu laden, die Windows uns anbietet, wenn wir keinen kompletten Pfad angeben.
		return LoadLibraryA(DBG_HELP_DLL_FILENAME);
	}
}

BS_MemoryDumper::BS_MemoryDumper() :
	m_DbghelpDLL(0)
{
	m_DbghelpDLL = LoadDbghelpDLL();
	if (m_DbghelpDLL)
	{
		m_MiniDumpWriteDump = reinterpret_cast<MINIDUMPWRITEDUMP>(GetProcAddress(m_DbghelpDLL, MINIDUMPWRITEDUMP_FUNCTIONNAME));
		if (!m_MiniDumpWriteDump)
		{
			BS_LOG_ERRORLN("Your version of \"%s\" is too old. Dumping is not possible.", DBG_HELP_DLL_FILENAME);
		}
	}
	else
	{
		BS_LOG_ERRORLN("Could not load \"%s\". Dumping is not possible.", DBG_HELP_DLL_FILENAME);
	}
}

// -----------------------------------------------------------------------------

BS_MemoryDumper::~BS_MemoryDumper()
{
	// DLL-Entladen
	if (m_DbghelpDLL) FreeLibrary(m_DbghelpDLL);
}


// -----------------------------------------------------------------------------

namespace
{
	bool EnsureOutputDirectoryExists()
	{
		// Windows.h definiert ein Makro CreateDirectory. Damit wir die Methode CreateDirectory aufrufen können müssen wir das Makro entfernen.
		#ifdef CreateDirectory
			#undef CreateDirectory
		#endif

		return BS_FileSystemUtil::GetInstance().CreateDirectory(
			BS_FileSystemUtil::GetInstance().GetUserdataDirectory() +
			BS_FileSystemUtil::GetInstance().GetPathSeparator() +
			DUMPS_DIRECTORYNAME);
	}

	string CreateOutputFilename()
	{
		// Aktuelle Zeit bestimmen.
		time_t Time = time(0);
		tm * Timeinfo = localtime(&Time);

		// Den Ausgabedateinamen in folgender Form erstellen:
		// <BenutzerdatenVerzeichnis>\dumps\YYYY-MM-DD HH-MM.dmp
		ostringstream oss;
		oss << BS_FileSystemUtil::GetInstance().GetUserdataDirectory() << BS_FileSystemUtil::GetInstance().GetPathSeparator()
			<< DUMPS_DIRECTORYNAME << BS_FileSystemUtil::GetInstance().GetPathSeparator()
			<< setfill('0')
			<< setw(4) << (Timeinfo->tm_year + 1900) << "-"
			<< setw(2) << (Timeinfo->tm_mon + 1) << "-"
			<< setw(2) << Timeinfo->tm_mday << " "
			<< setw(2) << Timeinfo->tm_hour << "-"
			<< setw(2) << Timeinfo->tm_min << "-"
			<< BS_Debugtools::GetVersionID()
			<< DUMPFILE_EXTENSION;
		return oss.str();
	}
}

bool BS_MemoryDumper::WriteDump(_EXCEPTION_POINTERS * ExceptionInfoPtr, string & Filename)
{
	// Dumpen ist nur möglich, wenn zuvor die dbghelp.dll geladen werden konnte.
	if (!m_DbghelpDLL)
	{
		BS_LOG_ERRORLN("Cannot write dump because \"%s\" could not be loaded previously.", DBG_HELP_DLL_FILENAME);
		return false;
	}

	// Temporäre Datei erstellen, die den Dump aufnehmen soll
	HANDLE File;
	Filename = CreateOutputFilename();
	if (!EnsureOutputDirectoryExists() ||
		(File = CreateFile(Filename.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0)) != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

		ExInfo.ThreadId = GetCurrentThreadId();
		ExInfo.ExceptionPointers = ExceptionInfoPtr;
		ExInfo.ClientPointers = 0;

		// Dump schreiben
		bool Result = m_MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), File, MiniDumpNormal, &ExInfo, 0, 0) != 0;
		if (!Result)
		{
			BS_LOG_ERRORLN("MiniDumpWriteDump() failed. Memory dump could not be created.");
			DeleteFileA(Filename.c_str());
		}

		CloseHandle(File);
		return Result;
	}
	else
	{
		BS_LOG_ERRORLN("Could not create a file to accomodate the memory dump.");
		return false;
	}
}
