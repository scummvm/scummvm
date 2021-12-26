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

#include "ags/shared/ac/common.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using namespace AGS::Shared;

const char *game_file_sig = "Adventure Creator Game File v2";

void quit(const String &str) {
	quit(str.GetCStr());
}

void quitprintf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	String text = String::FromFormatV(fmt, ap);
	va_end(ap);

	// WORKAROUND: In ScummVM we have to make this an error, because
	// too many places calling it presume it doesn't return,
	// and will throw a wobbly if does
	error("%s", text.GetCStr());
}

} // namespace AGS3
