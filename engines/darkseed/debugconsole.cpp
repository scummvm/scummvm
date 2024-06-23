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

#include "darkseed.h"
#include "darkseed/debugconsole.h"

namespace Darkseed {

DebugConsole::DebugConsole(TosText *tosText) : GUI::Debugger(), tosText(tosText) {
	registerCmd("tostext",   WRAP_METHOD(DebugConsole, Cmd_tostext));
	registerCmd("dt",   WRAP_METHOD(DebugConsole, Cmd_dt));
	registerCmd("getvar",   WRAP_METHOD(DebugConsole, Cmd_getvar));
	registerCmd("setvar",   WRAP_METHOD(DebugConsole, Cmd_setvar));
}

DebugConsole::~DebugConsole() {
}

bool DebugConsole::Cmd_tostext(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: tostext <index>\n");
		return true;
	}

	uint16 textIdx = atoi(argv[1]);
	if (textIdx < tosText->getNumEntries()) {
		debugPrintf("%s\n", tosText->getText(textIdx).c_str());
	} else {
		debugPrintf("index too large!\n");
	}
	return true;
}

bool DebugConsole::Cmd_dt(int argc, const char **argv) {
	int hour = g_engine->_currentTimeInSeconds / 60 / 60 + 1;
	debugPrintf("Day %d at %d:%02d%s (%d seconds)\n",
				g_engine->_currentDay,
				hour % 12,
				(g_engine->_currentTimeInSeconds / 60) % 60,
				hour < 12 ? "AM" : "PM", g_engine->_currentTimeInSeconds);
	return true;
}

bool DebugConsole::Cmd_getvar(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: getvar <index>\n");
		return true;
	}

	uint16 varIdx = atoi(argv[1]);
	debugPrintf("Object Var: %d\n", g_engine->_objects.getVar(varIdx));
	return true;
}

bool DebugConsole::Cmd_setvar(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: setvar <index> <newValue>\n");
		return true;
	}

	uint16 varIdx = atoi(argv[1]);
	int16 newValue = atoi(argv[2]);

	g_engine->_objects.setVar(varIdx, newValue);
	return true;
}

} // End of namespace Darkseed
