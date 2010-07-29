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

#include <stdio.h>
#include <stdarg.h>
#include <string>

#include "filesystemutil.h"
#include "log.h"
#include "debug/debugtools.h"

// Konstanten
static const char*	BF_LOG_FILENAME = "log.txt";
static const size_t	LOG_BUFFERSIZE = 1024 * 16;

// Logging soll nur stattfinden wenn es aktiviert ist
#ifdef BS_ACTIVATE_LOGGING

FILE*										BS_Log::_LogFile = NULL;
bool										BS_Log::_LineBegin = true;
const char*									BS_Log::_Prefix = NULL;
const char*									BS_Log::_File = NULL;
int											BS_Log::_Line = 0;
bool										BS_Log::_AutoNewline = false;
std::vector<BS_Log::LOG_LISTENER_CALLBACK>	BS_Log::_LogListener;

bool BS_Log::_CreateLog()
{
	// Logfile öffnen
	BS_FileSystemUtil::GetInstance().CreateDirectory(BS_FileSystemUtil::GetInstance().GetUserdataDirectory());
	_LogFile = fopen((BS_FileSystemUtil::GetInstance().GetUserdataDirectory() + "\\" + BF_LOG_FILENAME).c_str(), "w");

	if (_LogFile)
	{
		// Sicherstellen, dass es beim Beenden geschlossen wird
		atexit(_CloseLog);

		// Titelzeile in das Logfile schreiben
		Log("Broken Sword 2.5 Engine - Build: %s - %s - VersionID: %s\n", __DATE__, __TIME__, BS_Debugtools::GetVersionID());
		Log("-----------------------------------------------------------------------------------------------------\n");

		return true;
	}

	// Log-File konnte nicht erstellt werden
	return false;
}

void BS_Log::_CloseLog()
{
	if (_LogFile) fclose(_LogFile);
}

void BS_Log::Log(const char* Format, ...)
{
	char Message[LOG_BUFFERSIZE];

	// Nachricht erzeugen
	va_list ArgList;
	va_start(ArgList, Format);
	_vsnprintf(Message, sizeof(Message), Format, ArgList);

	// Nachricht loggen
	_WriteLog(Message);

	_FlushLog();
}

void BS_Log::LogPrefix(const char* Prefix, const char* Format, ...)
{
	char Message[LOG_BUFFERSIZE];
	char ExtFormat[LOG_BUFFERSIZE];

	// Falls die Ausgabe am Anfang einer neuen Zeile aufgehört hat, muss die neue Ausgabe mit dem Präfix
	// beginnen
	ExtFormat[0] = 0;
	if (_LineBegin)
	{
		_snprintf(ExtFormat, sizeof(ExtFormat), "%s%s: ", ExtFormat, Prefix);
		_LineBegin = false;
	}
	// Formatstring zeilenweise durchgehen und an jeden Zeilenanfang das Präfix setzen
	for (;;)
	{
		const char* NextLine = strstr(Format, "\n");
		if (!NextLine || *(NextLine + strlen("\n")) == 0)
		{
			_snprintf(ExtFormat, sizeof(ExtFormat), "%s%s", ExtFormat, Format);
			if (NextLine) _LineBegin = true;
			break;	
		}
		else
		{
			strncat(ExtFormat, Format, (NextLine - Format) + strlen("\n"));
			_snprintf(ExtFormat, sizeof(ExtFormat), "%s%s: ", ExtFormat, Prefix);
		}

		Format = NextLine + strlen("\n");
	}
	
	// Nachricht erzeugen
	va_list ArgList;
	va_start(ArgList, Format);
	_vsnprintf(Message, sizeof(Message), ExtFormat, ArgList);

	// Nachricht schreiben
	_WriteLog(Message);

	_FlushLog();
}

void BS_Log::LogDecorated(const char* Format, ...)
{
	// Nachricht erzeugen
	char Message[LOG_BUFFERSIZE];
	va_list ArgList;
	va_start(ArgList, Format);
	_vsnprintf(Message, sizeof(Message), Format, ArgList);

	// Zweiten Prefix erzeugen, falls gewünscht
	char SecondaryPrefix[1024];
	if (_File && _Line)
		_snprintf(SecondaryPrefix, sizeof(SecondaryPrefix), "(file: %s, line: %d) - ", _File, _Line);
	
	// Nachricht zeilenweise ausgeben und an jeden Zeilenanfang das Präfix setzen
	char* MessageWalker = Message;
	for (;;)
	{
		char* NextLine = strstr(MessageWalker, "\n");
		if (NextLine)
		{
			*NextLine = 0;
			if (_LineBegin)
			{
				_WriteLog(_Prefix);
				if (_File && _Line)
					_WriteLog(SecondaryPrefix);
			}
			_WriteLog(MessageWalker);
			_WriteLog("\n");
			MessageWalker = NextLine + sizeof("\n") - 1;
			_LineBegin = true;
		}
		else
		{
			if (_LineBegin)
			{
				_WriteLog(_Prefix);
				if (_File && _Line)
					_WriteLog(SecondaryPrefix);
			}
			_WriteLog(MessageWalker);
			_LineBegin = false;
			break;
		}
	}

	// Falls gewünscht, wird ans Ende der Nachricht automatisch ein Newline angehängt.
	if (_AutoNewline)
	{
		_WriteLog("\n");
		_LineBegin = true;
	}

	// Pseudoparameter zurücksetzen
	_Prefix = NULL;
	_File = 0;
	_Line = 0;
	_AutoNewline = false;

	_FlushLog();
}

int BS_Log::_WriteLog(const char* Message)
{
	if (!_LogFile) if (!_CreateLog()) return false;

	std::vector<LOG_LISTENER_CALLBACK>::iterator Iter = _LogListener.begin();
	for (; Iter != _LogListener.end(); ++Iter)
		(*Iter)(Message);

	fprintf(_LogFile, Message);

	return true;
}

void BS_Log::_FlushLog()
{
	fflush(_LogFile);
}

void (*BS_LogPtr)(const char *, ...) = BS_Log::Log;
extern "C"
{
	void BS_Log_C(const char* Message)
	{
		BS_LogPtr(Message);
	}
}

#else

extern "C"
{
	void BS_Log_C(const char* Message) {};
}

#endif
