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

#include "common/system.h"
#include "common/savefile.h"
#include "common/debug.h"
#include "bagel/boflib/log.h"

namespace Bagel {

static const char *const g_pszLogTypes[4] = {
	"Fatal Error: ",
	"Error: ",
	"Warning: ",
	""
};

void logInfo(const char *msg) {
	if (gDebugLevel > 0)
		debug("%s", msg);
}

void logWarning(const char *msg) {
	if (gDebugLevel > 0)
		debug("%s%s", g_pszLogTypes[2], msg);
}

void logError(const char *msg) {
	if (gDebugLevel > 0)
		debug("%s%s", g_pszLogTypes[1], msg);
}

const char *buildString(const char *pszFormat, ...) {
	static char szBuf[256];
	va_list argptr;

	assert(pszFormat != nullptr);

	if (pszFormat != nullptr) {
		// Parse the variable argument list
		va_start(argptr, pszFormat);
		Common::vsprintf_s(szBuf, pszFormat, argptr);
		va_end(argptr);

		return (const char *)&szBuf[0];
	}

	return nullptr;
}

} // namespace Bagel
