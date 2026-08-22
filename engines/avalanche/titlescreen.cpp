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

#include "avalanche/avalanche.h"
#include "avalanche/titlescreen.h"

namespace Avalanche {

TitleScreen::TitleScreen(AvalancheEngine *vm) : _vm(vm) {
}

TitleScreen::~TitleScreen() {
}

void TitleScreen::loadCredits(Common::Array<Common::String> &lines) {
	Common::File f;
	if (!f.open("credits.dat")) {
		warning("AVALANCHE: TitleScreen: File not found: credits.dat");
		return;
	}

	while (!f.eos()) {
		Common::String line;
		while (!f.eos()) {
			char ch = f.readByte();
			if (ch == '\n')
				break;
			if (ch != '\r')
				line += ch;
		}

		Common::String str;
		for (uint i = 0; i < line.size(); i++) {
			char c = line[i];
			if (c == '%')
				break;
			if (c == '@' || c == '^' || c == '>' || c == '*')
				continue;
			str += c;
		}
		str.trim();
		if (!str.empty())
			lines.push_back(str);
	}
	f.close();
}

void TitleScreen::run() {
	_vm->_sound->playMod("avalot2.mod");

	FontType avalotFont;
	Common::File fontFile;
	if (fontFile.open("avalot.fnt")) {
		for (int i = 0; i < 256; i++)
			fontFile.read(avalotFont[i], 16);
		fontFile.close();
	}

	_vm->_graphics->menuInitialize();
	Graphics::Surface &menu = _vm->_graphics->getMenuSurface();
	menu.fillRect(Common::Rect(0, 0, 640, 350), kColorBlack);

	CursorMan.showMouse(false);

	// Draw star arcs
	static const Color arcColors[5] = { kColorBlue, kColorLightblue, kColorCyan, kColorLightcyan, kColorWhite };
	byte c = 0;
	for (int gd = 1; gd <= 64; gd++) {
		for (int gm = 0; gm <= 60; gm++) {
			c = (c % 12) + 1;
			if (c > 5)
				continue;
			_vm->_graphics->drawArc(menu, 320, 175, gm * 6, gm * 6 + 1, gd * 6, arcColors[c - 1]);
		}
	}

	// Load logo.avd — 1bpp monochrome (each bit = white/black, written to all EGA planes simultaneously)
	// OR-paste at (112, 0): only white pixels land on screen, black are transparent
	Common::File logoFile;
	if (logoFile.open("logo.avd")) {
		Graphics::Surface logo;
		logo.create(53 * 8, 194 - 7 + 1, Graphics::PixelFormat::createFormatCLUT8());
		for (int y = 0; y < 194 - 7 + 1; y++) {
			for (int x = 0; x < 424; x += 8) {
				byte pixel = logoFile.readByte();
				for (int bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> (7 - bit)) & 1;
					*(byte *)logo.getBasePtr(x + bit, y) = pixelBit ? kColorWhite : kColorBlack;
				}
			}
		}
		logoFile.close();
		for (int y = 0; y < logo.h && y < menu.h; y++) {
			for (int x = 0; x < logo.w && (x + 112) < menu.w; x++) {
				byte color = *(byte *)logo.getBasePtr(x, y);
				// White pixels (logo) overwrite background (stars)
				if (color != kColorBlack)
					*(byte *)menu.getBasePtr(x + 112, y) = color;
			}
		}
		logo.free();
	}

	Common::Array<Common::String> credits;
	loadCredits(credits);

	Common::String scrollStr;
	for (uint i = 0; i < credits.size(); i++) {
		if (!scrollStr.empty())
			scrollStr += "  *  ";
		scrollStr += credits[i];
	}
	scrollStr += "   ";

	const int kBandCenterY = 315;
	const int kBandHalfH   = (110 - 50) / 3	;
	const int kBandTop     = kBandCenterY - kBandHalfH;
	const int kBandBottom     = kBandCenterY + kBandHalfH;

	// Phase 1: Gold/magenta band opening animation
	for (int spinnum = 50; spinnum <= 110 && !_vm->shouldQuit(); spinnum++) {
		int expand = (spinnum - 50) / 3;
		menu.fillRect(Common::Rect(0, kBandCenterY - expand, 640, kBandCenterY + expand + 1), kColorYellow);
		if (spinnum > 56) {
			int inner = (spinnum - 56) / 3;
			menu.fillRect(Common::Rect(0, kBandCenterY - inner, 640, kBandCenterY + inner + 1), kColorLightmagenta);
		}
		_vm->_graphics->menuRefreshScreen();
		g_system->delayMillis(3);

		Common::Event event;
		while (_vm->getEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
				_vm->_sound->stopMod();
				CursorMan.showMouse(true);
				return;
			}
		}
	}

	// Phase 2: Scroll credits text left across the band
	int scrollX = 640;
	uint32 lastTick = g_system->getMillis();

	while (!_vm->shouldQuit()) {
		// Draw settled band: gold & green borders + black center
		menu.fillRect(Common::Rect(0, kBandTop + 2, 640, kBandBottom - 1), kColorBlack);
		menu.fillRect(Common::Rect(0, kBandTop, 640, kBandTop + 2), kColorYellow);
		menu.fillRect(Common::Rect(0, kBandBottom - 1, 640, kBandBottom + 1), kColorYellow);
		menu.fillRect(Common::Rect(0, kBandTop + 2, 640, kBandTop + 4), kColorGreen);
		menu.fillRect(Common::Rect(0, kBandBottom - 3, 640, kBandBottom - 1), kColorGreen);

		int textY = kBandTop + (kBandHalfH * 2 - 16) / 2;
		_vm->_graphics->drawText(menu, scrollStr, avalotFont, 16, scrollX, textY, kColorWhite);
		_vm->_graphics->menuRefreshScreen();

		uint32 now = g_system->getMillis();
		if (now - lastTick >= 32) {
			scrollX -= 8;
			lastTick = now;
			if (scrollX < -(int)(scrollStr.size() * 8))
				break;
		}

		Common::Event event;
		while (_vm->getEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
				_vm->_sound->stopMod();
				CursorMan.showMouse(true);
				return;
			}
		}
		g_system->delayMillis(5);
	}

	_vm->_sound->stopMod();
	CursorMan.showMouse(true);
}

} // End of namespace Avalanche
