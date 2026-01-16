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

#include "console.h"

#include "pelrock/console.h"
#include "pelrock/types.h"

namespace Pelrock {

PelrockConsole::PelrockConsole(PelrockEngine *engine) : GUI::Debugger(), _engine(engine) {
	registerCmd("setScreen", WRAP_METHOD(PelrockConsole, cmdLoadRoom));
	registerCmd("give", WRAP_METHOD(PelrockConsole, cmdGiveItems));
}

PelrockConsole::~PelrockConsole() {
}

bool PelrockConsole::cmdLoadRoom(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: setScreen <roomNumber>");
		return true;
	}

	int roomNumber = atoi(argv[1]);
	g_engine->setScreen(roomNumber, ALFRED_DOWN);
	const WalkBox w = g_engine->_room->_currentRoomWalkboxes[0];
	g_engine->_alfredState.x = w.x;
	g_engine->_alfredState.y = w.y;
	debugPrintf("Loaded room %d", roomNumber);
	return true;
}

bool PelrockConsole::cmdGiveItems(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: giveItems <itemId> [itemId] ...");
		return true;
	}
	for (int i = 1; i < argc; i++) {
		int itemId = atoi(argv[i]);
		g_engine->_state->addInventoryItem(itemId);
		debugPrintf("Gave item %d\n", itemId);
	}
	return true;
}

} // End of namespace Pelrock
