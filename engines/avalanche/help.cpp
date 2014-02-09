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

/*
* This code is based on the original source code of Lord Avalot d'Argent version 1.3.
* Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
*/

/* Original name: HELPER	The help system unit. */

#include "avalanche/avalanche.h"
#include "avalanche/help.h"

namespace Avalanche {

Help::Help(AvalancheEngine *vm) {
	_vm = vm;

	_highlightWas = 0;
}

void Help::plotButton(int8 y, byte which) {
	warning("STUB: Help::plotButton()");
}

void Help::getMe(byte which) {
	warning("STUB: Help::getMe()");
}

Common::String Help::getLine() {
	warning("STUB: Help::getLine()");
	return "STUB: Help::getLine()";
}

byte Help::checkMouse() {
	warning("STUB: Help::checkMouse()");
	return 0;
}

void Help::continueHelp() {
	warning("STUB: Help::continueHelp()");
}

/**
 * @remarks Originally called 'boot_help'
 */
void Help::run() {
	warning("STUB: Help::run()");
}

} // End of namespace Avalanche
