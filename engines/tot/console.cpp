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

#include "tot/console.h"

namespace Tot {

TotConsole::TotConsole(TotEngine *engine) : _engine(engine) {
	registerCmd("jumpToPart2", WRAP_METHOD(TotConsole, cmdJumpToPart2));
	registerCmd("loadRoom", WRAP_METHOD(TotConsole, cmdLoadRoom));
	registerCmd("showMouseGrid", WRAP_METHOD(TotConsole, cmdShowMouseGrid));
	registerCmd("showGameGrid", WRAP_METHOD(TotConsole, cmdShowGameGrid));
	registerCmd("showScreenGrid", WRAP_METHOD(TotConsole, cmdShowScreenGrid));
	registerCmd("showObjectAreas", WRAP_METHOD(TotConsole, cmdShowObjectAreas));
	registerCmd("clearLayers", WRAP_METHOD(TotConsole, cmdClearLayers));
}

TotConsole::~TotConsole() {
}

bool TotConsole::cmdShowMouseGrid(int argc, const char **argv) {
	_engine->_showMouseGrid = true;
	debugPrintf("Enabled mouse hotspot grid");
	return true;
}

bool TotConsole::cmdShowGameGrid(int argc, const char **argv) {
	_engine->_showGameGrid = true;
	debugPrintf("Enabled screen base grid");
	return true;
}

bool TotConsole::cmdShowScreenGrid(int argc, const char **argv) {
	_engine->_showScreenGrid = true;
	debugPrintf("Enabled screen walk area grid");
	return true;
}

bool TotConsole::cmdShowObjectAreas(int argc, const char **argv) {
	_engine->_drawObjectAreas = true;
	debugPrintf("Enabled room object area display");
	return true;
}

bool TotConsole::cmdClearLayers(int argc, const char **argv) {
	_engine->_drawObjectAreas = false;
	_engine->_showScreenGrid = false;
	_engine->_showGameGrid = false;
	_engine->_showMouseGrid = false;
	g_engine->_graphics->drawScreen(g_engine->_sceneBackground);
	debugPrintf("Cleared all debug layers");
	return true;
}

bool TotConsole::cmdLoadRoom(int argc, const char **argv) {

	int roomID = atoi(argv[1]);

	if (roomID < 0 || roomID > 24) {
		debugPrintf("Invalid RoomID %d!\n", roomID);
		return true;
	}
	g_engine->clearAnimation();
	g_engine->clearScreenLayers();
	g_engine->loadScreenData(roomID);
	g_engine->_graphics->drawScreen(g_engine->_sceneBackground);
	debugPrintf("Loaded screen %d", roomID);
	return true;
}

bool TotConsole::cmdJumpToPart2(int argc, const char **argv) {
	g_engine->_list1Complete = true;
	g_engine->_list2Complete = true;
	debugPrintf("Moving on to part 2 of the game");
	return true;
}

} // End of namespace Tot
