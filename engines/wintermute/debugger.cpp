/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/wintermute/debugger.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

Console::Console(WintermuteEngine *vm) : GUI::Debugger(), _engineRef(vm) {
	DCmd_Register("show_fps", WRAP_METHOD(Console, Cmd_ShowFps));
}

Console::~Console(void) {

}

bool Console::Cmd_ShowFps(int argc, const char **argv) {
	if (argc > 1) {
		if (Common::String(argv[1]) == "true") {
			_engineRef->_game->_debugShowFPS = true;
		} else if (Common::String(argv[1]) == "false") {
			_engineRef->_game->_debugShowFPS = false;
		}
	}
	return true;
}
	
} // end of namespace Wintermute
