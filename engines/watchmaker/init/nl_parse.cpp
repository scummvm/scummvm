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

#define FORBIDDEN_SYMBOL_EXCEPTION_vsprintf

#include "watchmaker/init/nl_parse.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Watchmaker {

unsigned int jStringLimit = J_MAXSTRLEN, jTillEOL = 0, jUsingComments = 0;
unsigned int nlLineCounter;
char nlCurFileName[200];
static Common::SeekableReadStream *nlCurFile;

// PELS: supporto per il parsing dei .nl **********************************
static void (*ErrorFunc)(void) = nullptr;

int ParseError(const char *ln, ...) {
	char err[J_MAXSTRLEN];
	va_list ap;
	if (ErrorFunc)
		ErrorFunc();

	va_start(ap, ln);
	vsprintf(err, ln, ap);
	va_end(ap);
	warning("%s,%d: %s", nlCurFileName, nlLineCounter, err);
	return 0;
}

} // End of namespace Watchmaker
