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

/*
* This code is based on the original source code of Lord Avalot d'Argent version 1.3.
* Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
*/

#include "avalanche/avalanche.h"
#include "avalanche/mainmenu.h"

namespace Avalanche {

MainMenu::MainMenu(AvalancheEngine *vm) {
	_vm = vm;

	_registrant = Common::String("");
}

void MainMenu::run() {
	_vm->_graphics->menuInitialize();
	_vm->_graphics->menuLoadPictures();
	loadRegiInfo();
	loadFont();

	option(1, "Play the game.");
	option(2, "Read the background.");
	option(3, "Preview... perhaps...");
	option(4, "View the documentation.");
	option(5, "Registration info.");
	option(6, "Exit back to DOS.");
	centre(275, _registrant);
	centre(303, "Make your choice, or wait for the demo.");

	_vm->_graphics->menuRefreshScreen();

	wait();
	_vm->_graphics->menuClear();
}

void MainMenu::loadFont() {
	Common::File file;
	if (!file.open("avalot.fnt"))
		error("AVALANCHE: Scrolls: File not found: avalot.fnt");
	for (int16 i = 0; i < 256; i++)
		file.read(_font[i], 16);
	file.close();
}

void MainMenu::loadRegiInfo() {
	warning("STUB: MainMenu::loadRegiInfo()");
}

void MainMenu::option(byte which, Common::String what) {
	_vm->_graphics->menuDrawBigText(_font, 127, 39 + which * 33, Common::String(which + 48) + ')', true);
	_vm->_graphics->menuDrawBigText(_font, 191, 39 + which * 33, what, true);
}

void MainMenu::centre(int16 y, Common::String text) {
	warning("STUB: MainMenu::centre()");
}

void MainMenu::wait() {
	warning("STUB: MainMenu::wait()");
}

} // End of namespace Avalanche
