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

#include "m4/riddle/console.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {

Console::Console() : M4::Console() {
	registerCmd("global", WRAP_METHOD(Console, cmdGlobal));
}

bool Console::cmdGlobal(int argc, const char **argv) {
	if (argc == 2) {
		int flagNum = atol(argv[1]);
		debugPrintf("Global %d = %d\n", flagNum, _G(flags)[flagNum]);
	} else if (argc == 3) {
		int flagNum = atol(argv[1]);
		int flagVal = atol(argv[2]);
		_G(flags)[flagNum] = flagVal;
		debugPrintf("Global set\n");
	} else {
		debugPrintf("Global <num> [<value>]\n");
	}

	return true;
}

} // End of namespace Riddle
} // End of namespace M4
