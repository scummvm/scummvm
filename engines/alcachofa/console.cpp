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

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
#include "gui/console.h"
#endif

#include "alcachofa/console.h"
#include "alcachofa/script.h"
#include "alcachofa/alcachofa.h"

using namespace Common;

namespace Alcachofa {

Console::Console() : GUI::Debugger() {
	registerVar("showGraphics", &_showGraphics);
	registerVar("showInteractables", &_showInteractables);
	registerVar("showCharacters", &_showCharacters);
	registerVar("showFloorShape", &_showFloor);
	registerVar("showFloorEdges", &_showFloorEdges);
	registerVar("showFloorColor", &_showFloorColor);

	registerCmd("var", WRAP_METHOD(Console, cmdVar));
	registerCmd("processes", WRAP_METHOD(Console, cmdProcesses));
	registerCmd("room", WRAP_METHOD(Console, cmdRoom));
	registerCmd("rooms", WRAP_METHOD(Console, cmdRooms));
	registerCmd("changeRoom", WRAP_METHOD(Console, cmdChangeRoom));
	registerCmd("disableDebugDraw", WRAP_METHOD(Console, cmdDisableDebugDraw));
	registerCmd("pickup", WRAP_METHOD(Console, cmdItem));
	registerCmd("drop", WRAP_METHOD(Console, cmdItem));
	registerCmd("debugMode", WRAP_METHOD(Console, cmdDebugMode));
	registerCmd("tp", WRAP_METHOD(Console, cmdTeleport));
	registerCmd("toggleRoomFloor", WRAP_METHOD(Console, cmdToggleRoomFloor));
	registerCmd("playVideo", WRAP_METHOD(Console, cmdPlayVideo));
}

Console::~Console() {}

bool Console::isAnyDebugDrawingOn() const {
	return
		g_engine->isDebugModeActive() ||
		_showGraphics ||
		_showInteractables ||
		_showCharacters ||
		_showFloor ||
		_showFloorEdges ||
		_showFloorColor;
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
	} else if (argc == 2) {
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
	} else {
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
	} else if (g_engine->world().getRoomByName(args[1]) == nullptr)
		debugPrintf("Invalid room name: %s\n", args[1]);
	else {
		g_engine->player().changeRoom(args[1], true);
		return false;
	}
	return true;
}

bool Console::cmdDisableDebugDraw(int argc, const char **args) {
	_showInteractables = _showCharacters = _showFloor = _showFloorColor = false;
	return true;
}

bool Console::cmdItem(int argc, const char **args) {
	auto &inventory = g_engine->world().inventory();
	auto &mortadelo = g_engine->world().mortadelo();
	auto &filemon = g_engine->world().filemon();
	auto *active = g_engine->player().activeCharacter();
	if (argc < 2 || argc > 3) {
		debugPrintf("usage: %s [Mortadelo/Filemon] [<item>]\n\n", args[0]);
		debugPrintf("%20s%10s%10s\n", "Item", "Mortadelo", "Filemon");
		for (auto itItem = inventory.beginObjects(); itItem != inventory.endObjects(); ++itItem) {
			if (dynamic_cast<const Item *>(*itItem) == nullptr)
				continue;
			debugPrintf("%20s%10s%10s\n",
				(*itItem)->name().c_str(),
				mortadelo.hasItem((*itItem)->name()) ? "YES" : "no",
				filemon.hasItem((*itItem)->name()) ? "YES" : "no");
		}
		return true;
	}
	if (argc == 2 && active == nullptr) {
		debugPrintf("No character is active, name has to be specified\n");
		return true;
	}

	const char *itemName = args[1];
	if (argc == 3) {
		itemName = args[2];
		if (scumm_stricmp(args[1], "mortadelo") == 0 || scumm_stricmp(args[1], "m") == 0)
			active = &mortadelo;
		else if (scumm_stricmp(args[1], "filemon") == 0 || scumm_stricmp(args[1], "f") == 0)
			active = &filemon;
		else {
			debugPrintf("Invalid character name \"%s\", has to be either \"mortadelo\" or \"filemon\"\n", args[1]);
			return true;
		}
	}

	bool hasMatchedSomething = false;
	for (auto itItem = inventory.beginObjects(); itItem != inventory.endObjects(); ++itItem) {
		if (dynamic_cast<const Item *>(*itItem) == nullptr ||
			!(*itItem)->name().matchString(itemName, true))
			continue;
		hasMatchedSomething = true;
		if (args[0][0] == 'p')
			active->pickup((*itItem)->name(), false);
		else
			active->drop((*itItem)->name());
	}
	if (!hasMatchedSomething)
		debugPrintf("Cannot find any item matching \"%s\"\n", itemName);
	return true;
}

bool Console::cmdDebugMode(int argc, const char **args) {
	if (argc < 2 || argc > 3) {
		debugPrintf("usage: debugMode <mode> [<param>]\n");
		debugPrintf("modes:\n");
		debugPrintf("  0 - None, disables debug mode\n");
		debugPrintf("  1 - Closest floor point, param limits to polygon\n");
		debugPrintf("  2 - Floor edge intersections, param limits to polygon\n");
		debugPrintf("  3 - Teleport character to mouse click, param selects character\n");
		debugPrintf("  4 - Show floor alpha, param selects index of floor color object\n");
		debugPrintf("  5 - Show floor color, param selects index of floor color object\n");
		return true;
	}

	int32 param = -1;
	if (argc > 2) {
		char *end = nullptr;
		param = (int32)strtol(args[2], &end, 10);
		if (end == nullptr || *end != '\0') {
			debugPrintf("Debug mode parameter can only be integers");
			return true;
		}
	}

	auto mode = (DebugMode)strtol(args[1], nullptr, 10);
	g_engine->setDebugMode(mode, param);
	return true;
}

bool Console::cmdTeleport(int argc, const char **args) {
	if (argc < 1 || argc > 2) {
		debugPrintf("usage: tp [<character>]\n");
		debugPrintf("characters:\n");
		debugPrintf("  0 - Both\n");
		debugPrintf("  1 - Mortadelo\n");
		debugPrintf("  2 - Filemon\n");
	}

	int32 param = 0;
	if (argc > 1) {
		char *end = nullptr;
		param = (int32)strtol(args[1], &end, 10);
		if (end == nullptr || *end != '\0') {
			debugPrintf("Character kind can only be an integer\n");
			return true;
		}
	}

	g_engine->setDebugMode(DebugMode::TeleportCharacter, param);
	return false;
}

bool Console::cmdToggleRoomFloor(int argc, const char **args) {
	auto room = g_engine->player().currentRoom();
	if (room == nullptr) {
		debugPrintf("No room is active");
		return true;
	}

	room->toggleActiveFloor();
	return false;
}

bool Console::cmdPlayVideo(int argc, const char **args) {
	if (argc == 2) {
		char *end = nullptr;
		int32 videoId = (int32)strtol(args[1], &end, 10);
		if (end == nullptr || *end != '\0') {
			debugPrintf("Video ID can only be an integer\n");
			return true;
		}

#ifndef USE_TEXT_CONSOLE_FOR_DEBUGGER
		// we have to close the console *now* to properly see the video
		_debuggerDialog->close();
		g_system->clearOverlay();
#endif

		g_engine->playVideo(videoId);
		return false;
	} else
		debugPrintf("usage: playVideo <id>\n");
	return true;
}

} // End of namespace Alcachofa
