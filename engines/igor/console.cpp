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

#include "igor/console.h"
#include "igor/igor.h"

namespace Igor {

Console::Console() : GUI::Debugger() {
	registerCmd("test",   WRAP_METHOD(Console, Cmd_test));
	registerCmd("paint_walk",     WRAP_METHOD(Console, Cmd_paintWalk));
	registerCmd("paint_hotspots", WRAP_METHOD(Console, Cmd_paintHotspots));
	registerCmd("paint_off",      WRAP_METHOD(Console, Cmd_paintOff));
}

Console::~Console() {
}

bool Console::Cmd_test(int argc, const char **argv) {
	debugPrintf("Test\n");
	return true;
}

bool Console::Cmd_paintWalk(int argc, const char **argv) {
	if (!g_engine) {
		debugPrintf("Engine not running\n");
		return true;
	}
	debugPrintf("Overlay: walk areas (BOX .area != 0); colours are the area id\n");
	g_engine->debugPaintWalkAreas();
	return true;
}

bool Console::Cmd_paintHotspots(int argc, const char **argv) {
	if (!g_engine) {
		debugPrintf("Engine not running\n");
		return true;
	}
	debugPrintf("Overlay: hotspots (BOX .object != 0); colours are the object id\n");
	g_engine->debugPaintHotspots();
	return true;
}

bool Console::Cmd_paintOff(int argc, const char **argv) {
	if (!g_engine) {
		debugPrintf("Engine not running\n");
		return true;
	}
	g_engine->debugClearOverlay();
	debugPrintf("Overlay off\n");
	return true;
}

} // End of namespace Igor
