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
#include "m4/m4.h"
#include "m4/vars.h"
#include "m4/burger/vars.h"
#include "m4/burger/burger.h"

namespace M4 {

Console::Console() : GUI::Debugger() {
	registerCmd("teleport",  WRAP_METHOD(Console, cmdTeleport));
	registerCmd("global",    WRAP_METHOD(Console, cmdGlobal));
	registerCmd("item",      WRAP_METHOD(Console, cmdItem));
	registerCmd("hyperwalk", WRAP_METHOD(Console, cmdHyperwalk));
	registerCmd("digi",      WRAP_METHOD(Console, cmdDigi));
	registerCmd("trigger",   WRAP_METHOD(Console, cmdTrigger));
}

bool Console::cmdTeleport(int argc, const char **argv) {
	if (argc == 2) {
		_G(game).setRoom(atol(argv[1]));
		_G(kernel).teleported_in = true;
		return false;
	} else {
		debugPrintf("Currently in room %d\n", _G(game).room_id);
		return true;
	}
}

bool Console::cmdGlobal(int argc, const char **argv) {
	if (!Burger::g_vars) {
		debugPrintf("Not Orion Burger\n");
	} else if (argc == 2) {
		int flagNum = atol(argv[1]);
		debugPrintf("Global %d = %d\n", flagNum, Burger::g_vars->_flags[flagNum]);
	} else if (argc == 3) {
		int flagNum = atol(argv[1]);
		int flagVal = atol(argv[2]);
		Burger::g_vars->_flags[flagNum] = flagVal;
		debugPrintf("Global set\n");
	} else {
		debugPrintf("Global <num> [<value>]\n");
	}

	return true;
}

bool Console::cmdItem(int argc, const char **argv) {
	if (argc == 2) {
		inv_give_to_player(argv[1]);
		return false;
	} else {
		debugPrintf("item <item name>\n");
		return true;
	}
}

bool Console::cmdHyperwalk(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("hyperwalk [on | off]\n");
	} else {
		_G(hyperwalk) = !strcmp(argv[1], "on");
		debugPrintf("hyperwalk is now %s\n", _G(hyperwalk) ? "on" : "off");
	}

	return true;
}

bool Console::cmdDigi(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("digi <sound name>\n");
		return true;
	} else {
		digi_play(argv[1], 1);
		return false;
	}

	return true;
}

bool Console::cmdTrigger(int argc, const char **argv) {
	if (argc == 2) {
		kernel_trigger_dispatch_now(atol(argv[1]));
		return false;
	} else {
		debugPrintf("trigger <number>\n");
		return true;
	}
}

} // End of namespace M4
