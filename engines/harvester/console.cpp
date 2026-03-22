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

#include "harvester/console.h"
#include "harvester/harvester.h"

namespace Harvester {

Console::Console() : GUI::Debugger() {
	registerCmd("about", WRAP_METHOD(Console, Cmd_about));
	registerCmd("DEBUG_ROOM", WRAP_METHOD(Console, Cmd_debugRoom));
}

Console::~Console() {
}

bool Console::Cmd_about(int argc, const char **argv) {
	debugPrintf("Harvester engine scaffold\n");
	return true;
}

bool Console::Cmd_debugRoom(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: DEBUG_ROOM\n");
		return true;
	}

	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	const bool enabled = g_engine->toggleRoomDebugEnabled();
	debugPrintf("Room debug overlay %s\n", enabled ? "enabled" : "disabled");
	return true;
}

} // End of namespace Harvester
