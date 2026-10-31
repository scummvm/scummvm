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

#include "scooby/console.h"
#include "scooby/scooby.h"

namespace Scooby {

Console::Console(const ApplicationLoop *applicationLoop) : GUI::Debugger(), _applicationLoop(applicationLoop) {
	registerCmd("test", WRAP_METHOD(Console, Cmd_test));
	registerCmd("info", WRAP_METHOD(Console, Cmd_info));
	registerCmd("showWalkArea", WRAP_METHOD(Console, Cmd_showWalkArea));
}

Console::~Console() {
}

bool Console::Cmd_test(int argc, const char **argv) {
	debugPrintf("Test\n");
	return true;
}

bool Console::Cmd_info(int argc, const char **argv) {
	debugPrintf("RoomId: %d\n", _applicationLoop->getState().RoomId);
	return true;
}

bool Console::Cmd_showWalkArea(int argc, const char **argv) {
	// Toggles the walk-area overlay: a green checkerboard over the room points Shaggy can walk to from where he
	// stands (RoomVBlankHandler::updateWalkAreaOverlay).
	g_engine->setWalkAreaShown(!g_engine->isWalkAreaShown());
	debugPrintf("Walk area overlay %s\n", g_engine->isWalkAreaShown() ? "on" : "off");
	return true;
}

} // End of namespace Scooby
