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

#include "mads/madsv2/console.h"
#include "mads/madsv2/core/kernel.h"

namespace MADS {
namespace MADSV2 {

Console::Console() : GUI::Debugger() {
	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

bool Console::cmdTeleport(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Current room is: %d\n", new_room);
		debugPrintf("Usage: %s <room number>\n", argv[0]);
		return true;
	} else {
		new_room = strToInt(argv[1]);
		return false;
	}
}

} // namespace MADSV2
} // namespace MADS
