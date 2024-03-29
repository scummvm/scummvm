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
#include "bagel/boflib/error.h"

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
	"CRC failure",
	"Not enough disk space",
	"Future Use #15", // for future use
	"Future Use #16", // for future use
	"Future Use #17"  // for future use
};

// Static members
//
int CBofError::m_nErrorCount;
ERROR_CODE CBofError::m_errGlobal;


CBofError::CBofError() {
	m_errCode = ERR_NONE;
}

void CBofError::initialize() {
	m_nErrorCount = 0;
	m_errGlobal = ERR_NONE;
}

void CBofError::ReportError(ERROR_CODE errCode, const char *format, ...) {
	if ((m_errCode = errCode) != ERR_NONE) {
		Common::String buf;

		// Set global last error
		SetLastError(errCode);

		// One more error
		m_nErrorCount++;

		// Don't parse the variable input if there isn't any
		if (format != nullptr) {
			// Parse the arguments
			va_list argptr;
			va_start(argptr, format);
			buf = Common::String::vformat(format, argptr);
			va_end(argptr);
		}

		// Tell user about error, unless there were too many errors
		if (m_nErrorCount < MAX_ERRORS)
			bofMessageBox(buf, g_errList[errCode]);
	}
}

} // namespace Bagel
