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

#include "common/scummsys.h"
#include "backends/platform/sdl/riscos/riscos-utils.h"

#include <unixlib/local.h>

namespace RISCOS_Utils {

Common::String toRISCOS(Common::String path) {
	char start[PATH_MAX];
	char *end = __riscosify_std(path.c_str(), 0, start, PATH_MAX, 0);
	return Common::String(start, end);
}

Common::String toUnix(Common::String path) {
	Common::String out = Common::String(path);
	uint32 start = 0;
	if (out.contains("$")) {
		char *x = strstr(out.c_str(), "$");
		start = x ? x - out.c_str() : -1;
	} else if (out.contains(":")) {
		char *x = strstr(out.c_str(), ":");
		start = x ? x - out.c_str() : -1;
	}

	for (uint32 ptr = start; ptr < out.size(); ptr += 1) {
		switch (out.c_str()[ptr]) {
		case '.':
			out.setChar('/', ptr);
			break;
		case '/':
			out.setChar('.', ptr);
			break;
		case '\xA0':
			out.setChar(' ', ptr);
			break;
		default:
			break;
		}
	}

	if (out.contains("$") || out.contains(":"))
		out = "/" + out;

	return out;
}

}
