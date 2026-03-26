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

#include "common/algorithm.h"
#include "harvester/harvester.h"

namespace Harvester {

namespace {

static Script *getActiveStartupScript() {
	return g_engine ? g_engine->getStartupScript() : nullptr;
}

static void collectSortedRoomNames(const Script &script, Common::Array<Common::String> &roomNames) {
	roomNames.clear();

	for (const StartupRoomRecord &room : script.getRooms()) {
		bool alreadyPresent = false;
		for (const Common::String &existingName : roomNames) {
			if (existingName.equalsIgnoreCase(room.roomName)) {
				alreadyPresent = true;
				break;
			}
		}
		if (!alreadyPresent)
			roomNames.push_back(room.roomName);
	}

	Common::sort(roomNames.begin(), roomNames.end(), [](const Common::String &lhs, const Common::String &rhs) {
		Common::String upperLhs = lhs;
		Common::String upperRhs = rhs;
		upperLhs.toUppercase();
		upperRhs.toUppercase();
		return upperLhs < upperRhs;
	});
}

static bool findRoomName(const Script &script, const Common::String &candidate, Common::String &roomName) {
	for (const StartupRoomRecord &room : script.getRooms()) {
		if (!room.roomName.equalsIgnoreCase(candidate))
			continue;

		roomName = room.roomName;
		return true;
	}

	return false;
}

} // End of anonymous namespace

Console::Console() : GUI::Debugger() {
	registerCmd("about", WRAP_METHOD(Console, Cmd_about));
	registerCmd("DEBUG_COMBAT", WRAP_METHOD(Console, Cmd_debugCombat));
	registerCmd("DEBUG_ROOM", WRAP_METHOD(Console, Cmd_debugRoom));
	registerCmd("GOTO_ROOM", WRAP_METHOD(Console, Cmd_gotoRoom));
}

Console::~Console() {
}

bool Console::Cmd_about(int argc, const char **argv) {
	debugPrintf("Harvester engine scaffold\n");
	return true;
}

bool Console::Cmd_debugCombat(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: DEBUG_COMBAT\n");
		return true;
	}

	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	const bool enabled = g_engine->toggleCombatDebugEnabled();
	debugPrintf("Combat debug overlay %s\n", enabled ? "enabled" : "disabled");
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

bool Console::Cmd_gotoRoom(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Usage: GOTO_ROOM [room_name]\n");
		return true;
	}

	Script *startupScript = getActiveStartupScript();
	if (!startupScript) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	Common::Array<Common::String> roomNames;
	collectSortedRoomNames(*startupScript, roomNames);
	if (roomNames.empty()) {
		debugPrintf("No room targets are available\n");
		return true;
	}

	if (argc == 1) {
		debugPrintf("Available room targets:\n");
		for (const Common::String &roomName : roomNames)
			debugPrintf("  %s\n", roomName.c_str());
		return true;
	}

	if (!g_engine || !g_engine->hasCurrentStartupSaveRoomState()) {
		debugPrintf("GOTO_ROOM is only available while a room is active\n");
		return true;
	}

	Common::String roomName;
	if (!findRoomName(*startupScript, argv[1], roomName)) {
		debugPrintf("Unknown room target '%s'\n", argv[1]);
		return true;
	}

	if (!g_engine->requestDebugRoomChange(roomName)) {
		debugPrintf("Unable to queue room change to '%s'\n", roomName.c_str());
		return true;
	}

	debugPrintf("Queued room change to %s\n", roomName.c_str());
	return true;
}

} // End of namespace Harvester
