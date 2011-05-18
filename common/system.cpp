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
 */

// Disable symbol overrides so that we can use system headers.
// FIXME: Necessary for the PS2 port, should get rid of this eventually.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/system.h"
#include "common/str.h"

#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	#include "backends/platform/ps2/fileio.h"

	#define fputs(str, file)	ps2_fputs(str, file)
	#define fflush(a)			ps2_fflush(a)
#endif

#ifdef __DS__
	#include "backends/fs/ds/ds-fs.h"

	#define fputs(str, file)	DS::std_fwrite(str, strlen(str), 1, file)
	#define fflush(file)		DS::std_fflush(file)
#endif

OSystem *g_system = 0;

OSystem::OSystem() {
}

OSystem::~OSystem() {
}

void OSystem::fatalError() {
	quit();
	exit(1);
}

void OSystem::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);
}

Common::String OSystem::getSystemLanguage() const {
	return "en_US";
}
