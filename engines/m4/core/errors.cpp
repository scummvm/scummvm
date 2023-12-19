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

#include "common/file.h"
#include "common/textconsole.h"
#include "m4/core/errors.h"

namespace M4 {

inline static bool quadchar_equals_string(uint32 code, const Common::String &str) {
	return READ_BE_UINT32(str.c_str()) == code;
}

void error_show(const char *filename, uint32 line, quadchar errorcode, const char *fmt, ...) {
	assert(fmt);

	va_list va;
	va_start(va, fmt);
	Common::String msg = Common::String::vformat(fmt, va);
	va_end(va);

	error("%s", msg.c_str());
}

void error_show(const char *filename, uint32 line, quadchar errorcode) {
	error_show(filename, line, errorcode, "No extra description");
}

void error_look_up(quadchar errorcode, char *result_string) {
	Common::File f;
	*result_string = '\0';

	if (!f.open(ERROR_FILE))
		return;

	Common::String buffer;

	while (!f.eos()) {
		buffer = f.readString();
		const char *mark = buffer.c_str() + 1;

		if (quadchar_equals_string(errorcode, buffer) || quadchar_equals_string(errorcode, mark)) {
			const char *src = (const char *)buffer.c_str() + 5;
			int16 count = 0;

			do {
				*result_string++ = *src;
				++count;
			} while (*src++ && (count < MAX_STRING_LEN));

			break;
		}
	}
}

} // namespace M4
