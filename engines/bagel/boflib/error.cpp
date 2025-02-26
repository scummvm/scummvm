/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/str.h"
#include "gui/debugger.h"
#include "bagel/boflib/error.h"
#include "bagel/bagel.h"

namespace Bagel {

const char *const g_errList[] = {
	"No Error",
	"Out of memory",
	"Error opening file",
	"Error closing file",
	"Error reading file",
	"Error writing file",
	"Error seeking file",
	"Error deleting file",
	"Could not find file",
	"Invalid file type",
	"Invalid Path",
	"Disk error",
	"Unknown Error",
	"CRC failure"
};

// Static members
//
int CBofError::_count;


CBofError::CBofError() {
	_errCode = ERR_NONE;
}

void CBofError::initialize() {
	_count = 0;
}

void CBofError::reportError(ErrorCode errCode, const char *format, ...) {
	_errCode = errCode;

	if (_errCode == ERR_NONE)
		return;

	Common::String buf;

	// One more error
	_count++;

	// Don't parse the variable input if there isn't any
	if (format != nullptr) {
		// Parse the arguments
		va_list argptr;
		va_start(argptr, format);
		buf = Common::String::vformat(format, argptr);
		va_end(argptr);
	}

	// Tell user about error, unless there were too many errors
	if (_count < MAX_ERRORS)
		bofMessageBox(buf, g_errList[errCode]);

	GUI::Debugger *console = g_engine->getDebugger();
	if (console->isActive())
		console->debugPrintf("%s\n", buf.c_str());
}

void CBofError::fatalError(ErrorCode errCode, const char *format, ...) {
	Common::String buf;

	// Don't parse the variable input if there isn't any
	if (format != nullptr) {
		// Parse the arguments
		va_list argptr;
		va_start(argptr, format);
		buf = Common::String::vformat(format, argptr);
		va_end(argptr);
	}

	error("%s - %s", g_errList[errCode], buf.c_str());
}

} // namespace Bagel
