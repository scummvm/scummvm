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

#ifndef SWORD25_LOG_H
#define SWORD25_LOG_H

// Includes
#include "sword25/kernel/memlog_off.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"

// Logging soll nur stattfinden wenn es aktiviert ist
#ifdef BS_ACTIVATE_LOGGING

// Logging-Makros
#define BS_LOG				BS_Log::SetPrefix(BS_LOG_PREFIX ": "), BS_Log::LogDecorated
#define BS_LOGLN			BS_Log::SetPrefix(BS_LOG_PREFIX ": "), BS_Log::SetAutoNewline(true), BS_Log::LogDecorated
#define BS_LOG_WARNING		BS_Log::SetPrefix(BS_LOG_PREFIX ": WARNING - "), BS_Log::LogDecorated
#define BS_LOG_WARNINGLN	BS_Log::SetPrefix(BS_LOG_PREFIX ": WARNING - "), BS_Log::SetAutoNewline(true), BS_Log::LogDecorated
#define BS_LOG_ERROR		BS_Log::SetPrefix(BS_LOG_PREFIX ": ERROR - "), BS_Log::LogDecorated
#define BS_LOG_ERRORLN		BS_Log::SetPrefix(BS_LOG_PREFIX ": ERROR - "), BS_Log::SetAutoNewline(true), BS_Log::LogDecorated
#define BS_LOG_EXTERROR		BS_Log::SetPrefix(BS_LOG_PREFIX ": ERROR "), BS_Log::SetFile(__FILE__), BS_Log::SetLine(__LINE__), BS_Log::LogDecorated
#define BS_LOG_EXTERRORLN	BS_Log::SetPrefix(BS_LOG_PREFIX ": ERROR "), BS_Log::SetFile(__FILE__), BS_Log::SetLine(__LINE__),  BS_Log::SetAutoNewline(true), BS_Log::LogDecorated

// Die Version der Logging-Klasse mit aktiviertem Logging
class BS_Log
{
public:
	static void Clear();
	static void Log(const char* Format, ...);
	static void LogPrefix(const char* Prefix, const char* Format, ...);
	static void LogDecorated(const char* Format, ...);

	static void SetPrefix(const char* Prefix) { _Prefix = Prefix; }
	static void SetFile(const char* File) { _File = File; }
	static void SetLine(int Line) { _Line = Line; }
	static void SetAutoNewline(bool AutoNewline) { _AutoNewline = AutoNewline; }

	typedef void (*LOG_LISTENER_CALLBACK)(const char *);
	static void RegisterLogListener(LOG_LISTENER_CALLBACK Callback) { _LogListener.push_back(Callback); }
	static bool IsListenerRegistered(LOG_LISTENER_CALLBACK Callback) { return std::find(_LogListener.begin(), _LogListener.end(), Callback) != _LogListener.end(); }

private:
	static FILE*								_LogFile;
	static bool									_LineBegin;
	static const char*							_Prefix;
	static const char*							_File;
	static int									_Line;
	static bool									_AutoNewline;
	static std::vector<LOG_LISTENER_CALLBACK>	_LogListener;
	
	static bool _CreateLog();
	static void _CloseLog();

	static int _WriteLog(const char* Message);
	static void _FlushLog();
};

// Hilfsfunktion, die es C-Funktionen ermöglicht zu loggen (wird für Lua gebraucht).
extern "C"
{
	void BS_Log_C(const char* Message);
}

#else

// Logging-Makros
#define BS_LOG
#define BS_LOGLN
#define BS_LOG_WARNING
#define BS_LOG_WARNINGLN
#define BS_LOG_ERROR
#define BS_LOG_ERRORLN
#define BS_LOG_EXTERROR
#define BS_LOG_EXTERRORLN

// Die Version der Logging-Klasse mit deaktiviertem Logging
class BS_Log
{
public:
	// Die Log Funktionen werden zu do-nothing Funktionen und wird daher vom Compiler (hoffentlich) rausoptimiert
	static void Log(const char* Text, ...) {};
	static void LogPrefix(const char* Prefix, const char* Format, ...) {};
	static void LogDecorated(const char* Format, ...) {};

	static void SetPrefix(const char* Prefix) {};
	static void SetFile(const char* File) {};
	static void SetLine(int Line) {};
	static void SetAutoNewline(bool AutoNewline) {};

	typedef void (*LOG_LISTENER_CALLBACK)(const char *);
	static void RegisterLogListener(LOG_LISTENER_CALLBACK Callback) {};
};

extern "C"
{
	void BS_Log_C(const char* Message);
}

#endif

#endif
