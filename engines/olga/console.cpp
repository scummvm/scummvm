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

#include "olga/console.h"
#include "olga/olga.h"

namespace Olga {

Console::Console() : GUI::Debugger() {
	registerCmd("room", WRAP_METHOD(Console, Cmd_room));
}

Console::~Console() {
}

bool Console::Cmd_room(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Showing %s\n", g_engine->getRoomName().c_str());
		debugPrintf("Usage: room <LBM name without .LBM>, e.g. room klassen\n");
		return true;
	}
	if (!g_engine->showRoom(argv[1]))
		debugPrintf("Cannot show %s\n", argv[1]);
	return false;
}

} // End of namespace Olga
