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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "touche/console.h"
#include "touche/touche.h"

namespace Touche {

ToucheConsole::ToucheConsole(ToucheEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("startMusic", WRAP_METHOD(ToucheConsole, Cmd_StartMusic));
	registerCmd("stopMusic", WRAP_METHOD(ToucheConsole, Cmd_StopMusic));
}

ToucheConsole::~ToucheConsole() {
}

bool ToucheConsole::Cmd_StartMusic(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: startMusic <num>\n");
		return true;
	}

	int num = atoi(argv[1]);

	_vm->startMusic(num);
	return false;
}

bool ToucheConsole::Cmd_StopMusic(int argc, const char **argv) {
	_vm->stopMusic();
	return false;
}

} // End of namespace Touche
