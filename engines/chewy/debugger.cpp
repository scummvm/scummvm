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
#include "chewy/debugger.h"
#include "chewy/global.h"
#include "chewy/ngshext.h"

namespace Chewy {

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
		::error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

Debugger::Debugger() : GUI::Debugger() {
	registerCmd("room", WRAP_METHOD(Debugger, Cmd_GotoRoom));
	registerCmd("item", WRAP_METHOD(Debugger, Cmd_Item));
}

Debugger::~Debugger() {
}

bool Debugger::Cmd_GotoRoom(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("%s [roomNum]\n", argv[0]);
		return true;
	} else {
		int roomNum = strToInt(argv[1]);
		exit_room(-1);
		_G(spieler).PersonRoomNr[P_CHEWY] = roomNum;
		room->load_room(&room_blk, roomNum, &_G(spieler));
		fx_blend = BLEND1;
		enter_room(-1);

		return false;
	}
}

bool Debugger::Cmd_Item(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("%s [itemNum]\n", argv[0]);
	} else {
		int itemNum = strToInt(argv[1]);
		invent_2_slot(itemNum);
		debugPrintf("Done.\n");
	}

	return true;
}

} // namespace Chewy
