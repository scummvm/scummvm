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

#include "sherlock/graphics.h"
#include "sherlock/sherlock.h"
#include "common/system.h"
#include "graphics/palette.h"

namespace Sherlock {

Surface::Surface(uint16 width, uint16 height) {
	create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

Surface::~Surface() {
    free();
}

void Surface::fillRect(int x1, int y1, int x2, int y2, byte color) {
    Graphics::Surface::fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void Surface::drawSprite(int x, int y, SpriteFrame *spriteFrame, bool flipped, bool altFlag) {
	
	
}

/*----------------------------------------------------------------*/

Screen::Screen(SherlockEngine *vm) : Surface(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), _vm(vm),
		_backBuffer1(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT),
		_backBuffer2(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT) {
	setFont(1);
}

void Screen::setFont(int fontNumber) {
	_fontNumber = fontNumber;
	Common::String fname = Common::String::format("FONT%d.VGS", fontNumber);
	Common::SeekableReadStream *stream = _vm->_res->load(fname);

	debug("TODO: Loading font %s, size - %d", fname.c_str(), stream->size());

	delete stream;
}

void Screen::update() {
	g_system->copyRectToScreen(getPixels(), this->w, 0, 0, this->w, this->h);
	g_system->updateScreen();
}

void Screen::getPalette(byte palette[PALETTE_SIZE]) {
	g_system->getPaletteManager()->grabPalette(palette, 0, PALETTE_COUNT);
}

void Screen::setPalette(const byte palette[PALETTE_SIZE]) {
	g_system->getPaletteManager()->setPalette(palette, 0, PALETTE_COUNT);
}

int Screen::equalizePalette(const byte palette[PALETTE_SIZE]) {
	int total = 0;
	byte tempPalette[PALETTE_SIZE];
	getPalette(tempPalette);

	// For any palette component that doesn't already match the given destination
	// palette, change by 1 towards the reference palette component
	for (int idx = 0; idx < PALETTE_SIZE; ++idx) {
		if (tempPalette[idx] > palette[idx])
		{
			--tempPalette[idx];
			++total;
		} else if (tempPalette[idx] < palette[idx]) {
			++tempPalette[idx];
			++total;
		}
	}

	if (total > 0)
		// Palette changed, so reload it
		setPalette(tempPalette);

	return total;
}

void Screen::fadeToBlack() {
	const int FADE_AMOUNT = 2;
	bool repeatFlag;
	byte *srcP;
	int count;
	byte tempPalette[PALETTE_SIZE];

	getPalette(tempPalette);
	do {
		repeatFlag = false;
		for (srcP = &tempPalette[0], count = 0; count < PALETTE_SIZE; ++count, ++srcP) {
			int v = *srcP;
			if (v) {
				repeatFlag = true;
				*srcP = MAX(*srcP - FADE_AMOUNT, 0);
			}
		}

		setPalette(tempPalette);
		_vm->_events->pollEventsAndWait();
	} while (repeatFlag && !_vm->shouldQuit());
}

} // End of namespace Sherlock
