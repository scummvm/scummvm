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

#include "sword25/kernel/log.h"
#include "base/version.h"
#include "common/config-manager.h"
#include "common/fs.h"

namespace Sword25 {

static const char *BF_LOG_FILENAME = "log.txt";
static const size_t LOG_BUFFERSIZE = 1024 * 16;

// Logging will take place only when it's activated
#ifdef BS_ACTIVATE_LOGGING

Common::WriteStream *BS_Log::_logFile = NULL;
bool BS_Log::_lineBegin = true;
const char *BS_Log::_prefix = NULL;
const char *BS_Log::_file = NULL;
int BS_Log::_line = 0;
bool BS_Log::_autoNewline = false;

bool BS_Log::createLog() {
	// Open the log file
	Common::FSNode dataDir(ConfMan.get("path"));
	Common::FSNode file = dataDir.getChild(BF_LOG_FILENAME);

	// Open the file for saving
	_logFile = file.createWriteStream();

	if (_logFile) {
		// Add a title into the log file
		log("Broken Sword 2.5 Engine - Build: %s - %s - VersionID: %s\n", __DATE__, __TIME__, gScummVMFullVersion);
		log("-----------------------------------------------------------------------------------------------------\n");

		return true;
	}

	// Log file could not be created
	return false;
}

void BS_Log::closeLog() {
	delete _logFile;
	_logFile = NULL;
}

void BS_Log::log(const char *format, ...) {
	char message[LOG_BUFFERSIZE];

	// Create the message
	va_list argList;
	va_start(argList, format);
	vsnprintf(message, sizeof(message), format, argList);

	// Log the message
	writeLog(message);

	flushLog();
}

void BS_Log::logPrefix(const char *prefix, const char *format, ...) {
	char message[LOG_BUFFERSIZE];
	char extFormat[LOG_BUFFERSIZE];

	// If the issue has ceased at the beginning of a new line, the new issue to begin with the prefix
	extFormat[0] = 0;
	if (_lineBegin) {
		snprintf(extFormat, sizeof(extFormat), "%s: ", prefix);
		_lineBegin = false;
	}
	// Format String pass line by line and each line with the initial prefix
	for (;;) {
		const char *nextLine = strstr(format, "\n");
		if (!nextLine || *(nextLine + strlen("\n")) == 0) {
			Common::strlcat(extFormat, format, sizeof(extFormat));
			if (nextLine)
				_lineBegin = true;
			break;
		} else {
			strncat(extFormat, format, (nextLine - format) + strlen("\n"));
			Common::strlcat(extFormat, prefix, sizeof(extFormat));
			Common::strlcat(extFormat, ": ", sizeof(extFormat));
		}

		format = nextLine + strlen("\n");
	}

	// Create message
	va_list argList;
	va_start(argList, format);
	vsnprintf(message, sizeof(message), extFormat, argList);

	// Log the message
	writeLog(message);

	flushLog();
}

void BS_Log::logDecorated(const char *format, ...) {
	// Nachricht erzeugen
	char message[LOG_BUFFERSIZE];
	va_list argList;
	va_start(argList, format);
	vsnprintf(message, sizeof(message), format, argList);

	// Zweiten prefix erzeugen, falls gewünscht
	char secondaryPrefix[1024];
	if (_file && _line)
		snprintf(secondaryPrefix, sizeof(secondaryPrefix), "(file: %s, line: %d) - ", _file, _line);

	// Nachricht zeilenweise ausgeben und an jeden Zeilenanfang das Präfix setzen
	char *messageWalker = message;
	for (;;) {
		char *nextLine = strstr(messageWalker, "\n");
		if (nextLine) {
			*nextLine = 0;
			if (_lineBegin) {
				writeLog(_prefix);
				if (_file && _line)
					writeLog(secondaryPrefix);
			}
			writeLog(messageWalker);
			writeLog("\n");
			messageWalker = nextLine + sizeof("\n") - 1;
			_lineBegin = true;
		} else {
			if (_lineBegin) {
				writeLog(_prefix);
				if (_file && _line)
					writeLog(secondaryPrefix);
			}
			writeLog(messageWalker);
			_lineBegin = false;
			break;
		}
	}

	// Falls gewünscht, wird ans Ende der Nachricht automatisch ein Newline angehängt.
	if (_autoNewline) {
		writeLog("\n");
		_lineBegin = true;
	}

	// Pseudoparameter zurücksetzen
	_prefix = NULL;
	_file = 0;
	_line = 0;
	_autoNewline = false;

	flushLog();
}

int BS_Log::writeLog(const char *message) {
	if (!_logFile && !createLog())
		return false;

	debugN(0, "%s", message);

	_logFile->writeString(message);

	return true;
}

void BS_Log::flushLog() {
	if (_logFile)
		_logFile->flush();
}

void (*BS_LogPtr)(const char *, ...) = BS_Log::log;

void BS_Log_C(const char *message) {
	BS_LogPtr(message);
}

#else

void BS_Log_C(const char *message) {};

#endif

} // End of namespace Sword25
