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
#define BS_LOG              BS_Log::setPrefix(BS_LOG_PREFIX ": "), BS_Log::logDecorated
#define BS_LOGLN            BS_Log::setPrefix(BS_LOG_PREFIX ": "), BS_Log::setAutoNewline(true), BS_Log::logDecorated
#define BS_LOG_WARNING      BS_Log::setPrefix(BS_LOG_PREFIX ": WARNING - "), BS_Log::logDecorated
#define BS_LOG_WARNINGLN    BS_Log::setPrefix(BS_LOG_PREFIX ": WARNING - "), BS_Log::setAutoNewline(true), BS_Log::logDecorated
#define BS_LOG_ERROR        BS_Log::setPrefix(BS_LOG_PREFIX ": ERROR - "), BS_Log::logDecorated
#define BS_LOG_ERRORLN      BS_Log::setPrefix(BS_LOG_PREFIX ": ERROR - "), BS_Log::setAutoNewline(true), BS_Log::logDecorated
#define BS_LOG_EXTERROR     BS_Log::setPrefix(BS_LOG_PREFIX ": ERROR "), BS_Log::setFile(__FILE__), BS_Log::setLine(__LINE__), BS_Log::logDecorated
#define BS_LOG_EXTERRORLN   BS_Log::setPrefix(BS_LOG_PREFIX ": ERROR "), BS_Log::setFile(__FILE__), BS_Log::setLine(__LINE__),  BS_Log::setAutoNewline(true), BS_Log::logDecorated

// Die Version der Logging-Klasse mit aktiviertem Logging
class BS_Log {
public:
	static void clear();
	static void log(const char *format, ...);
	static void logPrefix(const char *prefix, const char *format, ...);
	static void logDecorated(const char *format, ...);

	static void setPrefix(const char *prefix) {
		_prefix = prefix;
	}
	static void setFile(const char *file) {
		_file = file;
	}
	static void setLine(int line) {
		_line = line;
	}
	static void setAutoNewline(bool autoNewline) {
		_autoNewline = autoNewline;
	}

	static void closeLog();

private:
	static Common::WriteStream *_logFile;
	static bool _lineBegin;
	static const char *_prefix;
	static const char *_file;
	static int _line;
	static bool _autoNewline;

	static bool createLog();

	static int writeLog(const char *message);
	static void flushLog();
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
	static void log(const char *text, ...) {};
	static void logPrefix(const char *prefix, const char *format, ...) {};
	static void logDecorated(const char *format, ...) {};

	static void setPrefix(const char *prefix) {};
	static void setFile(const char *file) {};
	static void setLine(int line) {};
	static void setAutoNewline(bool autoNewline) {};

	typedef void (*LOG_LISTENER_CALLBACK)(const char *);
	static void registerLogListener(LOG_LISTENER_CALLBACK callback) {};
};

#define BS_Log_C error

#endif

} // End of namespace Sword25

#endif
