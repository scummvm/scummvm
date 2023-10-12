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

#include "m4/console.h"
#include "m4/vars.h"
#include "m4/burger/vars.h"

namespace M4 {

Console::Console() : GUI::Debugger() {
	registerCmd("test",   WRAP_METHOD(Console, Cmd_test));
	registerCmd("room",   WRAP_METHOD(Console, Cmd_room));
	registerCmd("flag",   WRAP_METHOD(Console, Cmd_flag));
	registerCmd("item",   WRAP_METHOD(Console, Cmd_item));
	registerCmd("hyperwalk", WRAP_METHOD(Console, Cmd_hyperwalk));
}

Console::~Console() {
}

bool Console::Cmd_test(int argc, const char **argv) {
	debugPrintf("Test\n");
	return true;
}

bool Console::Cmd_room(int argc, const char **argv) {
	if (argc == 2) {
		_G(game).setRoom(atol(argv[1]));
		return false;
	} else {
		debugPrintf("room <room number>]\n");
		return true;
	}
}

bool Console::Cmd_flag(int argc, const char **argv) {
	if (!Burger::g_vars) {
		debugPrintf("Not Orion Burger\n");
	} else if (argc == 2) {
		int flagNum = atol(argv[1]);
		debugPrintf("Flag %d = %ld\n", flagNum, Burger::g_vars->_flags[flagNum]);
	} else if (argc == 3) {
		int flagNum = atol(argv[1]);
		int flagVal = atol(argv[2]);
		Burger::g_vars->_flags[flagNum] = flagVal;
		debugPrintf("Flag set\n");
	} else {
		debugPrintf("Flag <num> [<value>]\n");
	}

	return true;
}

bool Console::Cmd_item(int argc, const char **argv) {
	if (argc == 2) {
		inv_give_to_player(argv[1]);
		return false;
	} else {
		debugPrintf("item <item name>\n");
		return true;
	}
}

bool Console::Cmd_hyperwalk(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("hyperwalk [on | off]\n");
	} else {
		_G(hyperwalk) = !strcmp(argv[1], "on");
		debugPrintf("hyperwalk is now %s\n", _G(hyperwalk) ? "on" : "off");
	}

	return true;
}

} // End of namespace M4
