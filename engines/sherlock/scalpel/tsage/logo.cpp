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

#include "common/scummsys.h"
#include "sherlock/scalpel/tsage/logo.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {
namespace Scalpel {
namespace TsAGE {

bool Logo::show(ScalpelEngine *vm) {
	Events &events = *vm->_events;
	Logo *logo = new Logo(vm);
	bool interrupted = false;

	while (!logo->finished()) {
		logo->nextFrame();

		events.wait(2);
		events.setButtonState();

		interrupted = vm->shouldQuit() || events.kbHit() || events._pressed;
		if (interrupted) {
			events.clearEvents();
			break;
		}
	}

	delete logo;
	return !interrupted;
}

Logo::Logo(ScalpelEngine *vm) : _vm(vm), _lib("sf3.rlb"), _surface(vm->_screen->w(), vm->_screen->h()) {
	// Initialize frame counter
	_counter = 0;
	
	// Set up the palettes
	Common::fill(&_palette1[0], &_palette1[PALETTE_SIZE], 0);
	Common::fill(&_palette1[0], &_palette2[PALETTE_SIZE], 0);
	Common::fill(&_palette1[0], &_palette3[PALETTE_SIZE], 0);

	_lib.getPalette(_palette1, 1111);
	_lib.getPalette(_palette1, 10);
	_lib.getPalette(_palette2, 1111);
	_lib.getPalette(_palette2, 1);
	_lib.getPalette(_palette3, 1111);
	_lib.getPalette(_palette3, 14);
}

bool Logo::finished() const {
	return false;
}

void Logo::nextFrame() {

	switch (_counter++) {
	case 0:
		loadBackground();
		fade(_palette1);
		break;

	default:
		break;
	}
}

void Logo::loadBackground() {
	Screen &screen = *_vm->_screen;
	
	for (int idx = 0; idx < 4; ++idx) {
		// Get the portion of the screen
		Common::SeekableReadStream *stream = _lib.getResource(RES_BITMAP, 10, idx);

		// Load it onto the surface
		Common::Point pt((idx / 2) * (_surface.w() / 2), (idx % 2) * (_surface.h() / 2));
		for (int y = 0; y < (_surface.h() / 2); ++y, ++pt.y) {
			byte *pDest = (byte *)_surface.getBasePtr(pt.x, pt.y);
			stream->read(pDest, _surface.w() / 2);
		}

		//	_backgroundBounds = Rect(0, 0, READ_LE_UINT16(data), READ_LE_UINT16(data + 2));
		delete stream;
	}

	// Default to a blank palette
	byte palette[PALETTE_SIZE];
	Common::fill(&palette[0], &palette[PALETTE_SIZE], 0);
	screen.setPalette(palette);

	// Copy the surface to the screen
	screen.blitFrom(_surface);
}

void Logo::fade(const byte palette[PALETTE_SIZE]) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	byte tempPalette[PALETTE_SIZE];

	for (int percent = 0; percent < 100; percent += 6) {
		for (int palIndex = 0; palIndex < 256; ++palIndex) {
			const byte *palP = (const byte *)&palette[palIndex * 3];
			byte *destP = &tempPalette[palIndex * 3];

			for (int rgbIndex = 0; rgbIndex < 3; ++rgbIndex, ++palP, ++destP) {
				*destP = (int)*palP * percent / 100;
			}
		}

		screen.setPalette(tempPalette);
		events.wait(1);
	}

	// Set final palette
	screen.setPalette(palette);
}

} // end of namespace TsAGE
} // end of namespace Scalpel
} // end of namespace Sherlock
