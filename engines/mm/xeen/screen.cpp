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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "mm/xeen/screen.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/window.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

Screen::Screen(XeenEngine *vm) : _vm(vm) {
	_fadeIn = false;
	create(SCREEN_WIDTH, SCREEN_HEIGHT);
	Common::fill(&_tempPalette[0], &_tempPalette[PALETTE_SIZE], 0);
	Common::fill(&_mainPalette[0], &_mainPalette[PALETTE_SIZE], 0);
}

void Screen::loadPalette(const Common::Path &name) {
	File f(name);
	for (int i = 0; i < PALETTE_SIZE; ++i)
		_tempPalette[i] = f.readByte() << 2;
}

void Screen::loadBackground(const Common::Path &name) {
	File f(name);

	assert(f.size() == (SCREEN_WIDTH * SCREEN_HEIGHT));
	f.read((byte *)getPixels(), SCREEN_WIDTH * SCREEN_HEIGHT);
	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

void Screen::loadPage(int pageNum) {
	assert(pageNum == 0 || pageNum == 1);
	if (_pages[0].empty()) {
		_pages[0].create(SCREEN_WIDTH, SCREEN_HEIGHT);
		_pages[1].create(SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	_pages[pageNum].blitFrom(*this);
}

void Screen::freePages() {
	_pages[0].free();
	_pages[1].free();
}

void Screen::horizMerge(int xp) {
	if (_pages[0].empty())
		return;

	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		byte *destP = (byte *)getBasePtr(0, y);
		const byte *srcP = (const byte *)_pages[0].getBasePtr(xp, y);
		Common::copy(srcP, srcP + SCREEN_WIDTH - xp, destP);

		if (xp != 0) {
			destP = (byte *)getBasePtr(SCREEN_WIDTH - xp, y);
			srcP = (const byte *)_pages[1].getBasePtr(0, y);
			Common::copy(srcP, srcP + xp, destP);
		}
	}

	markAllDirty();
}

void Screen::vertMerge(int yp) {
	if (_pages[0].empty())
		return;

	for (int y = 0; y < SCREEN_HEIGHT - yp; ++y) {
		const byte *srcP = (const byte *)_pages[0].getBasePtr(0, yp + y);
		byte *destP = (byte *)getBasePtr(0, y);
		Common::copy(srcP, srcP + SCREEN_WIDTH, destP);
	}

	for (int y = 0; y < yp; ++y) {
		const byte *srcP = (const byte *)_pages[1].getBasePtr(0, y);
		byte *destP = (byte *)getBasePtr(0, SCREEN_HEIGHT - yp + y);
		Common::copy(srcP, srcP + SCREEN_WIDTH, destP);
	}

	markAllDirty();
}

void Screen::drawScreen() {
	addDirtyRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

void Screen::fadeIn(int step) {
	_fadeIn = true;
	fadeInner(step);
}

void Screen::fadeOut(int step) {
	_fadeIn = false;
	fadeInner(step);
}

void Screen::fadeInner(int step) {
	for (int idx = 128; idx >= 0 && !_vm->shouldExit(); idx -= step) {
		int val = MAX(idx, 0);
		bool flag = !_fadeIn;
		if (!flag) {
			val = -(val - 128);
			flag = step != 0x81;
		}

		if (!flag) {
			step = 0x80;
		} else {
			// Create a scaled palette from the temporary one
			for (int i = 0; i < PALETTE_SIZE; ++i) {
				_mainPalette[i] = (_tempPalette[i] * val * 2) >> 8;
			}

			updatePalette();
		}

		_vm->_events->pollEventsAndWait();
	}

	update();
}

void Screen::updatePalette() {
	updatePalette(_mainPalette, 0, 16);
}

void Screen::updatePalette(const byte *pal, int start, int count16) {
	g_system->getPaletteManager()->setPalette(pal, start, count16 * 16);
}

void Screen::saveBackground(int slot) {
	assert(slot > 0 && slot < 10);
	_savedScreens[slot - 1].copyFrom(*this);
}

void Screen::restoreBackground(int slot) {
	assert(slot > 0 && slot < 10);

	blitFrom(_savedScreens[slot - 1]);
}

bool Screen::doScroll(bool rollUp, bool fadeInFlag) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	const int SCROLL_L[8] = { 29, 23, 15, -5, -11, -23, -49, -71 };
	const int SCROLL_R[8] = { 165, 171, 198, 218, 228, 245, 264, 281 };

	if (_vm->_files->_ccNum) {
		if (fadeInFlag)
			screen.fadeIn(2);
		return _vm->shouldExit();
	}

	screen.saveBackground();

	// Load hand sprites
	SpriteResource *hand[16];
	for (int i = 0; i < 16; ++i) {
		Common::Path name(Common::String::format("hand%02d.vga", i));
		hand[i] = new SpriteResource(name);
	}

	// Load marb sprites
	SpriteResource *marb[5];
	for (int i = 0; i < 4; ++i) {
		Common::Path name(Common::String::format("marb%02d.vga", i + 1));
		marb[i] = new SpriteResource(name);
	}

	if (rollUp) {
		for (int i = 22, ctr = 7; i > 0 && !events.isKeyMousePressed()
			&& !_vm->shouldExit(); --i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i > 14) {
				hand[14]->draw(0, 0, Common::Point(SCROLL_L[ctr], 0), SPRFLAG_800);
				hand[15]->draw(0, 0, Common::Point(SCROLL_R[ctr], 0), SPRFLAG_800);
				--ctr;
			} else if (i != 0) {
				hand[i - 1]->draw(0, 0);
			}

			if (i <= 20)
				marb[(i - 1) / 5]->draw(0, (i - 1) % 5);
			screen.update();

			while (!_vm->shouldExit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

			if (i == 0 && fadeInFlag)
				screen.fadeIn(2);
		}
	} else {
		for (int i = 0, ctr = 0; i < 22 && !events.isKeyMousePressed()
			&& !_vm->shouldExit(); ++i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i < 14) {
				hand[i]->draw(0, 0);
			} else {
				hand[14]->draw(0, 0, Common::Point(SCROLL_L[ctr], 0), SPRFLAG_800);
				hand[15]->draw(0, 0, Common::Point(SCROLL_R[ctr], 0), SPRFLAG_800);
				++ctr;
			}

			if (i < 20) {
				marb[i / 5]->draw(0, i % 5);
			}
			screen.update();

			while (!_vm->shouldExit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

			if (i == 0 && fadeInFlag)
				screen.fadeIn(2);
		}
	}

	if (rollUp) {
		hand[0]->draw(0, 0);
		marb[0]->draw(0, 0);
	} else {
		screen.restoreBackground();
	}
	screen.update();

	// Free resources
	for (int i = 0; i < 4; ++i)
		delete marb[i];
	for (int i = 0; i < 16; ++i)
		delete hand[i];

	return _vm->shouldExit() || events.isKeyMousePressed();
}

} // End of namespace Xeen
} // End of namespace MM
