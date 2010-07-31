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

#include "sword25/kernel/log.h"
#include "base/version.h"
#include "common/config-manager.h"
#include "common/fs.h"

namespace Sword25 {

// Constants
static const char *BF_LOG_FILENAME = "log.txt";
static const size_t	LOG_BUFFERSIZE = 1024 * 16;

// Logging will take place only when it's activated
#ifdef BS_ACTIVATE_LOGGING

Common::WriteStream*							BS_Log::_LogFile = NULL;
bool											BS_Log::_LineBegin = true;
const char *									BS_Log::_Prefix = NULL;
const char *									BS_Log::_File = NULL;
int												BS_Log::_Line = 0;
bool											BS_Log::_AutoNewline = false;
Common::Array<BS_Log::LOG_LISTENER_CALLBACK>	BS_Log::_LogListener;

bool BS_Log::_CreateLog() {
	// Open the log file
	Common::FSNode dataDir(ConfMan.get("path"));
	Common::FSNode file = dataDir.getChild(BF_LOG_FILENAME);

	// Open the file for saving
	_LogFile = file.createWriteStream();

	if (_LogFile) {
		// Add a title into the log file
		Log("Broken Sword 2.5 Engine - Build: %s - %s - VersionID: %s\n", __DATE__, __TIME__, gScummVMFullVersion);
		Log("-----------------------------------------------------------------------------------------------------\n");

		return true;
	}

	// Log file could not be created
	return false;
}

void BS_Log::_CloseLog() {
	delete _LogFile;
	_LogFile = NULL;
}

void BS_Log::Log(const char *Format, ...) {
	char Message[LOG_BUFFERSIZE];

	// Create the message
	va_list ArgList;
	va_start(ArgList, Format);
	_vsnprintf(Message, sizeof(Message), Format, ArgList);

	// Log the message
	_WriteLog(Message);

	_FlushLog();
}

void BS_Log::LogPrefix(const char *Prefix, const char *Format, ...) {
	char Message[LOG_BUFFERSIZE];
	char ExtFormat[LOG_BUFFERSIZE];

	// If the issue has ceased at the beginning of a new line, the new issue to begin with the prefix
	ExtFormat[0] = 0;
	if (_LineBegin) {
		_snprintf(ExtFormat, sizeof(ExtFormat), "%s%s: ", ExtFormat, Prefix);
		_LineBegin = false;
	}
	// Format String pass line by line and each line with the initial prefix
	for (;;) {
		const char *NextLine = strstr(Format, "\n");
		if (!NextLine || *(NextLine + strlen("\n")) == 0) {
			_snprintf(ExtFormat, sizeof(ExtFormat), "%s%s", ExtFormat, Format);
			if (NextLine) _LineBegin = true;
			break;	
		} else {
			strncat(ExtFormat, Format, (NextLine - Format) + strlen("\n"));
			_snprintf(ExtFormat, sizeof(ExtFormat), "%s%s: ", ExtFormat, Prefix);
		}

		Format = NextLine + strlen("\n");
	}
	
	// Create message
	va_list ArgList;
	va_start(ArgList, Format);
	_vsnprintf(Message, sizeof(Message), ExtFormat, ArgList);

	// Log the message
	_WriteLog(Message);

	_FlushLog();
}

void BS_Log::LogDecorated(const char *Format, ...) {
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
	char *MessageWalker = Message;
	for (;;) {
		char *NextLine = strstr(MessageWalker, "\n");
		if (NextLine) {
			*NextLine = 0;
			if (_LineBegin) {
				_WriteLog(_Prefix);
				if (_File && _Line)
					_WriteLog(SecondaryPrefix);
			}
			_WriteLog(MessageWalker);
			_WriteLog("\n");
			MessageWalker = NextLine + sizeof("\n") - 1;
			_LineBegin = true;
		} else {
			if (_LineBegin) {
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
	if (_AutoNewline) {
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

int BS_Log::_WriteLog(const char * Message) {
	if (!_LogFile) if (!_CreateLog()) return false;

	Common::Array<LOG_LISTENER_CALLBACK>::iterator Iter = _LogListener.begin();
	for (; Iter != _LogListener.end(); ++Iter)
		(*Iter)(Message);

	_LogFile->writeString(Message);

	return true;
}

void BS_Log::_FlushLog() {
	_LogFile->flush();
}

void (*BS_LogPtr)(const char *, ...) = BS_Log::Log;

void BS_Log_C(const char *Message) {
	BS_LogPtr(Message);
}

#else

void BS_Log_C(const char* Message) {};

#endif

} // End of namespace Sword25
