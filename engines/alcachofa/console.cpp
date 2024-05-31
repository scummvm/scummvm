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
#include "script.h"
#include "alcachofa.h"

using namespace Common;

namespace Alcachofa {

Console::Console() : GUI::Debugger() {
	registerVar("showInteractables", &_showInteractables);
	registerVar("showCharacters", &_showCharacters);
	registerVar("showFloor", &_showFloor);
	registerVar("showFloorColor", &_showFloorColor);

	registerCmd("var", WRAP_METHOD(Console, cmdVar));
	registerCmd("processes", WRAP_METHOD(Console, cmdProcesses));
	registerCmd("room", WRAP_METHOD(Console, cmdRoom));
	registerCmd("rooms", WRAP_METHOD(Console, cmdRooms));
	registerCmd("changeRoom", WRAP_METHOD(Console, cmdChangeRoom));
	registerCmd("disableDebugDraw", WRAP_METHOD(Console, cmdDisableDebugDraw));
}

Console::~Console() {
}

bool Console::cmdVar(int argc, const char **args) {
	auto &script = g_engine->script();
	if (argc < 2 || argc > 3)
		debugPrintf("usage: %s <name> [<value>]\n", args[0]);
	else if (argc == 3) {
		char *end = nullptr;
		int32 value = (int32)strtol(args[2], &end, 10);
		if (end == nullptr || *end != '\0')
			debugPrintf("Invalid variable value: %s", args[2]);
		else if (!script.hasVariable(args[1]))
			debugPrintf("Invalid variable name: %s", args[1]);
		else
			script.variable(args[1]) = value;
	}
	else if (argc == 2) {
		bool hadSomeMatch = false;
		for (auto it = script.beginVariables(); it != script.endVariables(); it++) {
			if (matchString(it->_key.c_str(), args[1], true)) {
				hadSomeMatch = true;
				debugPrintf("  %32s = %d\n", it->_key.c_str(), script.variable(it->_key.c_str()));
			}
		}
		if (!hadSomeMatch)
			debugPrintf("Could not find any variable with pattern: %s\n", args[1]);
	}
	return true;
}

bool Console::cmdProcesses(int argc, const char **args) {
	g_engine->scheduler().debugPrint();
	return true;
}

bool Console::cmdRoom(int argc, const char **args) {
	if (argc > 2) {
		debugPrintf("usage: %s [<name>]\n", args[0]);
		return true;
	}
	Room *room = nullptr;
	if (argc == 1) {
		room = g_engine->player().currentRoom();
		if (room == nullptr) {
			debugPrintf("Player is currently in no room, cannot print details\n");
			return true;
		}
	}
	else {
		room = g_engine->world().getRoomByName(args[1]);
		if (room == nullptr) {
			debugPrintf("Could not find room with exact name: %s\n", args[1]);
			return cmdRooms(argc, args);
		}
	}
	room->debugPrint(true);
	return true;
}

bool Console::cmdRooms(int argc, const char **args) {
	if (argc != 2) {
		debugPrintf("usage: %s <pattern>\n", args[0]);
		return true;
	}
	bool hadSomeMatch = false;
	for (auto it = g_engine->world().beginRooms(); it != g_engine->world().endRooms(); it++) {
		if ((*it)->name().matchString(args[1], true)) {
			hadSomeMatch = true;
			(*it)->debugPrint(false);
		}
	}
	if (!hadSomeMatch)
		debugPrintf("Could not find any room with pattern: %s\n", args[1]);
	return true;
}

bool Console::cmdChangeRoom(int argc, const char **args) {
	if (argc > 2)
		debugPrintf("usage: %s <name>\n", args[0]);
	else if (argc == 1) {
		Room *current = g_engine->player().currentRoom();
		debugPrintf("Current room: %s\n", current == nullptr ? "<null>" : current->name().c_str());
	}
	else if (g_engine->world().getRoomByName(args[1]) == nullptr)
		debugPrintf("Invalid room name: %s\n", args[1]);
	else
		g_engine->player().changeRoom(args[1], true);
	return true;
}

bool Console::cmdDisableDebugDraw(int argc, const char **args) {
	_showInteractables = _showCharacters = _showFloor = _showFloorColor = false;
	return true;
}

} // End of namespace Alcachofa
