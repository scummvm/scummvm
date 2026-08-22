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
			case Common::KEYCODE_4:
				showDoc("avalot.doc", 0, false);
				_vm->_graphics->menuInitialize();
				_vm->_graphics->menuLoadPictures();
				drawMenu();
				break;
			case Common::KEYCODE_5:
				showDoc("avalot.doc", 1094, true);
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

void MainMenu::showDoc(const Common::String &filename, int startLine, bool isRegi) {
	Common::File file;
	if (!file.open(filename.c_str())) {
		warning("AVALANCHE: File not found: %s", filename.c_str());
		return;
	}

	Common::Array<Common::String> lines;
	while (!file.eos()) {
		Common::String line;
		while (!file.eos()) {
			char ch = file.readByte();
			if (ch == '\n')
				break;
			if (ch == '\t') {
				int spaces = 8 - (line.size() % 8);
				for (int s = 0; s < spaces; s++)
					line += ' ';
			} else if ((byte)ch >= 32 && (byte)ch <= 126) {
				line += ch;
			}
		}
		lines.push_back(line);
	}
	file.close();

	if (lines.empty())
		return;

	Common::Array<DocSection> sections;
	DocSection startSec;
	startSec.name = "Start of documentation";
	startSec.line = 0;
	sections.push_back(startSec);

	for (int i = 1; i < (int)lines.size(); i++) {
		Common::String trimmed = lines[i];
		trimmed.trim();
		if (trimmed.size() >= 3 && trimmed[0] == '"' && trimmed[1] == '"' && trimmed[2] == '"') {
			Common::String headerStr = lines[i - 1];
			headerStr.trim();
			if (!headerStr.empty()) {
				DocSection sec;
				sec.name = headerStr;
				sec.line = i - 1;
				sections.push_back(sec);
			}
		}
	}

	int currentLine = startLine;
	_vm->_graphics->menuInitialize();
	Graphics::Surface &menu = _vm->_graphics->getMenuSurface();
	CursorMan.showMouse(false);

	if (!isRegi && startLine == 0) {
		// Show Table of Contents index screen
		int tocLine = showDocTOC(lines, sections);
		if (tocLine >= 0)
			currentLine = tocLine;
	}

	const int linesPerPage = 23;

	while (!_vm->shouldQuit()) {
		menu.fillRect(Common::Rect(0, 0, 640, 332), kColorBlue);

		for (int i = 0; i < linesPerPage; i++) {
			int lineIdx = currentLine + i;
			if (lineIdx >= (int)lines.size())
				break;

			Common::String line = lines[lineIdx];
			if (line.size() > 78)
				line = Common::String(line.c_str(), 78);

			_vm->_graphics->drawText(menu, line, _font, 14, 8, 4 + i * 14, kColorCyan);
		}

		int pct = (currentLine + linesPerPage) * 100 / lines.size();
		if (pct > 100)
			pct = 100;
		else if (pct <= 1)
			pct = 0;

		Common::String statusStr;
		if (isRegi)
			statusStr = Common::String::format("Doc lister: PgUp, PgDn, Home & End to move. Esc exits to main menu. | %d %% through", pct);
		else
			statusStr = Common::String::format("Doc lister: PgUp, PgDn, Home & End to move. Esc exits. C=contents |%d %% through", pct);

		menu.fillRect(Common::Rect(0, 332, 640, 350), kColorLightgray);
		_vm->_graphics->drawText(menu, statusStr, _vm->_font, 8, 4, 336, kColorBlack);

		_vm->_graphics->menuRefreshScreen();

		Common::Event event;
		bool redraw = false;
		while (_vm->getEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN) {
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					CursorMan.showMouse(true);
					return;
				case Common::KEYCODE_c:
					if (!isRegi) {
						int tocLine = showDocTOC(lines, sections);
						if (tocLine >= 0)
							currentLine = tocLine;
						redraw = true;
					}
					break;
				case Common::KEYCODE_PAGEUP:
				case Common::KEYCODE_UP:
					currentLine -= linesPerPage;
					if (currentLine < 0)
						currentLine = 0;
					redraw = true;
					break;
				case Common::KEYCODE_PAGEDOWN:
				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_SPACE:
					if (currentLine + linesPerPage < (int)lines.size())
						currentLine += linesPerPage;
					redraw = true;
					break;
				case Common::KEYCODE_HOME:
					currentLine = 0;
					redraw = true;
					break;
				case Common::KEYCODE_END:
					currentLine = (int)lines.size() - linesPerPage;
					if (currentLine < 0)
						currentLine = 0;
					redraw = true;
					break;
				default:
					break;
				}
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				CursorMan.showMouse(true);
				return;
			}
		}

		if (!redraw)
			g_system->delayMillis(15);
	}

	CursorMan.showMouse(true);
}

int MainMenu::showDocTOC(const Common::Array<Common::String> &lines, const Common::Array<DocSection> &sections) {
	Graphics::Surface &menu = _vm->_graphics->getMenuSurface();
	menu.fillRect(Common::Rect(0, 0, 640, 332), kColorBlue);

	Common::String header = "-=- The contents of the Lord AVALOT D'Argent (version 1.3) documentation -=-";
	_vm->_graphics->drawText(menu, header, _font, 14, 320 - (header.size() * 4), 4, kColorCyan);

	for (int i = 0; i < (int)sections.size() && i < 26; i++) {
		char keyChar = (i < 9) ? ('1' + i) : ('A' + (i - 9));
		Common::String itemStr = Common::String::format("%c. %s", keyChar, sections[i].name.c_str());
		if (itemStr.size() > 76)
			itemStr = Common::String(itemStr.c_str(), 76);

		int x = 12;
		int y = 20 + i * 11;

		_vm->_graphics->drawText(menu, itemStr, _vm->_font, 8, x, y, kColorCyan);
	}

	Common::String tocStatus = "Esc=to doc lister | Press the key listed next to the section you wish to jump to";
	menu.fillRect(Common::Rect(0, 332, 640, 350), kColorLightgray);
	_vm->_graphics->drawText(menu, tocStatus, _vm->_font, 8, 4, 338, kColorBlack);

	_vm->_graphics->menuRefreshScreen();

	while (!_vm->shouldQuit()) {
		Common::Event event;
		while (_vm->getEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return -1;

				char c = toupper(event.kbd.ascii);
				int targetSec = -1;
				if (c >= '1' && c <= '9')
					targetSec = c - '1';
				else if (c >= 'A' && c <= 'Z')
					targetSec = 9 + (c - 'A');

				if (targetSec >= 0 && targetSec < (int)sections.size())
					return sections[targetSec].line;
			}
		}
		g_system->delayMillis(15);
	}
	return -1;
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
