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

#include "m4/burger/console.h"
#include "m4/burger/vars.h"
#include "m4/burger/burger.h"

namespace M4 {
namespace Burger {

Console::Console() : M4::Console() {
	registerCmd("global", WRAP_METHOD(Console, cmdGlobal));
	registerCmd("test", WRAP_METHOD(Console, cmdTest));
	registerCmd("time", WRAP_METHOD(Console, cmdTime));
}

bool Console::cmdGlobal(int argc, const char **argv) {
	if (argc == 2) {
		int flagNum = atol(argv[1]);
		debugPrintf("Flag %d = %d\n", flagNum, _G(flags)[(Flag)flagNum]);
	} else if (argc == 3) {
		int flagNum = atol(argv[1]);
		int flagVal = atol(argv[2]);
		_G(flags)[(Flag)flagNum] = flagVal;
		debugPrintf("Flag set\n");
	} else {
		debugPrintf("global <num> [<value>]\n");
	}

	return true;
}

bool Console::cmdTest(int argc, const char **argv) {
	int tests = _G(flags)[kFirstTestPassed] ? 1 : 0 +
		_G(flags)[kSecondTestPassed] ? 1 : 0 +
		_G(flags)[kThirdTestPassed] ? 1 : 0 +
		_G(flags)[kFourthTestPassed] ? 1 : 0 +
		_G(flags)[kFifthTestPassed] ? 1 : 0;

	debugPrintf("Tests passed = %d\n", tests);
	return true;
}

bool Console::cmdTime(int argc, const char **argv) {
	if (argc == 2) {
		int newTime = atol(argv[1]);
		_G(flags).set_boonsville_time(newTime - 1);
		return false;

	} else {
		debugPrintf("Current time is %d\n", _G(flags)[kBoonsvilleTime]);
		return true;
	}
}

} // End of namespace Burger
} // End of namespace M4
