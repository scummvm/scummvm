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

#include "crab/crab.h"
#include "crab/console.h"
#include "crab/input/input.h"

namespace Crab {

using namespace pyrodactyl::input;

Console::Console() : GUI::Debugger() {
	registerCmd("draw", WRAP_METHOD(Console, cmdDraw));
	registerCmd("what", WRAP_METHOD(Console, cmdWhat));
}

Console::~Console() {
}

bool Console::cmdDraw(int argc, const char **argv) {
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (!scumm_stricmp(argv[i], "OFF"))
				g_engine->_debugDraw = 0;
			else if (!scumm_stricmp(argv[i], "TMX"))
				g_engine->_debugDraw |= DRAW_TMX;
			else if (!scumm_stricmp(argv[i], "PROPS"))
				g_engine->_debugDraw |= DRAW_PROP_BOUNDS;
			else if (!scumm_stricmp(argv[i], "SPRITE"))
				g_engine->_debugDraw |= DRAW_SPRITE_BOUNDS;
			else if (!scumm_stricmp(argv[i], "PATHING"))
				g_engine->_debugDraw |= DRAW_PATHING;
			else if (!scumm_stricmp(argv[i], "FPS"))
				g_engine->_debugDraw |= DRAW_FPS;
			else if (!scumm_stricmp(argv[i], "ALL"))
				g_engine->_debugDraw = DRAW_TMX | DRAW_PROP_BOUNDS | DRAW_SPRITE_BOUNDS | DRAW_PATHING | DRAW_FPS;
			else
				debugPrintf("Valid parameters are 'TMX', 'PROPS', 'SPRITE', 'PATHING', 'FPS', 'ALL' or 'OFF'\n");
		}
	}
	return true;
}

bool Console::cmdWhat(int argc, const char **argv) {
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (!scumm_stricmp(argv[i], "keymap")) {
				switch (g_engine->_inputManager->getKeyBindingMode()) {
				case KBM_NONE:
					debugPrintf("KBM_NONE\n");
					break;

				case KBM_GAME:
					debugPrintf("KBM_GAME\n");
					break;

				case KBM_UI:
					debugPrintf("KBM_UI\n");
					break;

				default:
					debugPrintf("Unknown KBM\n");
					break;
				}
			} else
				debugPrintf("Valid parameters are 'keymap'\n");
		}
	}
	return true;
}

} // End of namespace Crab
