/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_LOG_H
#define SWORD25_LOG_H

// Includes
#include "common/array.h"
#include "common/file.h"
#include "sword25/kernel/common.h"

namespace Sword25 {

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
	static bool IsListenerRegistered(LOG_LISTENER_CALLBACK Callback) { 
		Common::Array<LOG_LISTENER_CALLBACK>::iterator i;
		for (i = _LogListener.begin(); i != _LogListener.end(); ++i) {
			if (**i == Callback)
				return true;
		}
		return false;
	}
	static void _CloseLog();

private:
	static Common::WriteStream *				_LogFile;
	static bool									_LineBegin;
	static const char *							_Prefix;
	static const char *							_File;
	static int									_Line;
	static bool									_AutoNewline;
	static Common::Array<LOG_LISTENER_CALLBACK>	_LogListener;
	
	static bool _CreateLog();

	static int _WriteLog(const char* Message);
	static void _FlushLog();
};

// Auxiliary function that allows to log C functions (needed for Lua).
#define BS_Log_C error


#else

// Logging-Macros
#define BS_LOG
#define BS_LOGLN
#define BS_LOG_WARNING
#define BS_LOG_WARNINGLN
#define BS_LOG_ERROR
#define BS_LOG_ERRORLN
#define BS_LOG_EXTERROR
#define BS_LOG_EXTERRORLN

// The version of the logging class with logging disabled 
class BS_Log {
public:
	// This version implements all the various methods as empty stubs
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

#define BS_Log_C error

#endif

} // End of namespace Sword25

#endif
