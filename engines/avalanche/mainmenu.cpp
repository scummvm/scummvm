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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "common/system.h"

#include "avalanche/avalanche.h"
#include "avalanche/mainmenu.h"
#include "avalanche/intro.h"
#include "titlescreen.h"

namespace Avalanche {

MainMenu::MainMenu(AvalancheEngine *vm) {
	_vm = vm;

	_registrant = Common::String("");
}

void MainMenu::run() {
	_vm->_titleScreen->run();
	_vm->_graphics->menuInitialize();
	_vm->_graphics->menuLoadPictures();
	loadRegiInfo();
	loadFont();
	drawMenu();
	wait();
}

void MainMenu::drawMenu() {
	CursorMan.showMouse(false);
	option(1, "Play the game.");
	option(2, "Read the background.");
	option(3, "Preview... perhaps...");
	option(4, "View the documentation.");
	option(5, "Registration info.");
	option(6, "Exit back to DOS.");
	centre(274, _registrant);
	centre(301, "Make your choice, or wait for the demo.");

	_vm->_graphics->menuRefreshScreen();
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
	_registrant = "(Unregistered evaluation copy.)";
	warning("STUB: MainMenu::loadRegiInfo()");
}

void MainMenu::option(byte which, Common::String what) {
	_vm->_graphics->menuDrawBigText(_font, 127, 39 + which * 33, Common::String(which + 48) + ')', kColorBlack);
	_vm->_graphics->menuDrawBigText(_font, 191, 39 + which * 33, what, kColorBlack);
}

void MainMenu::centre(int16 y, Common::String text) {
	_vm->_graphics->menuDrawBigText(_font, 320 - text.size() * 8, y, text, kColorLightgray);
}

void MainMenu::wait() {
	int x = 0;
	while (!_vm->shouldQuit()) {
		_vm->_system->delayMillis(40);
		x++;
		if (x == 641)
			x = 0;
		Common::Event event;
		_vm->getEvent(event);
		if (event.type == Common::EVENT_KEYDOWN) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_1: // Falltroughs are inteded.
				// Play the game
				_vm->_graphics->menuFree();
				_vm->_graphics->menuRestoreScreen();
				CursorMan.showMouse(true);
				return;
			case Common::KEYCODE_2:
				_vm->_intro->run();
				_vm->_graphics->menuInitialize();
				_vm->_graphics->menuLoadPictures();
				drawMenu();
				break;
			case Common::KEYCODE_3:
				showPreview();
				_vm->_graphics->menuInitialize();
				_vm->_graphics->menuLoadPictures();
				drawMenu();
				break;
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_6: // Falltroughs are inteded.
				// Exit back to DOS
				_vm->_letMeOut = true;
				_vm->_graphics->menuFree();
				return;
			default:
				break;
			}
		}
	}
}

void MainMenu::showPreview() {
	Common::File file;
	if (!file.open("preview2.avd")) {
		warning("AVALANCHE: File not found: preview2.avd");
		return;
	}

	_vm->_graphics->menuInitialize();
	Graphics::Surface &preview = _vm->_graphics->getMenuSurface();
	preview.fillRect(Common::Rect(0, 0, 640, 350), kColorBlack);

	file.seek(177);

	byte planes[4][151 * 80];
	for (int plane = 0; plane < 4; plane++)
		file.read(planes[plane], 151 * 80);
	file.close();

	for (int y = 0; y < 151; y++) {
		int destY1 = 10 + (y * 4) / 3;
		int destY2 = 10 + ((y + 1) * 4) / 3;
		for (int x = 0; x < 640; x++) {
			int byteIdx = y * 80 + (x / 8);
			int bitIdx = 7 - (x % 8);

			byte color = 0;
			for (int plane = 0; plane < 4; plane++) {
				byte bit = (planes[plane][byteIdx] >> bitIdx) & 1;
				color |= (bit << plane);
			}
			for (int dy = destY1; dy < destY2 && dy < preview.h; dy++)
				*(byte *)preview.getBasePtr(x, dy) = color;
		}
	}

	_vm->_graphics->drawText(preview, "...This is a preview of things to come...", _font, 16, 320 - 20 * 8, 225, kColorCyan);
	_vm->_graphics->drawText(preview, "AVAROID", _font, 16, 320 - 4 * 8, 245, kColorYellow);
	_vm->_graphics->drawText(preview, "(a space so dizzy)", _font, 16, 320 - 9 * 8, 263, kColorYellow);
	_vm->_graphics->drawText(preview, "the next Avvy adventure-- in 256 colours.", _font, 16, 320 - 20 * 8, 283, kColorLightblue);
	_vm->_graphics->drawText(preview, "Any key...", _font, 16, 540, 318, kColorLightgray);

	_vm->_graphics->menuRefreshScreen();
	CursorMan.showMouse(false);

	uint32 startTime = g_system->getMillis();
	while (!_vm->shouldQuit()) {
		if (g_system->getMillis() - startTime >= 15000)
			break;

		Common::Event event;
		while (_vm->getEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
				CursorMan.showMouse(true);
				return;
			}
		}
		g_system->delayMillis(10);
	}

	CursorMan.showMouse(true);
}

} // End of namespace Avalanche
