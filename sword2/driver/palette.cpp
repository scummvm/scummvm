/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/driver/d_draw.h"

namespace Sword2 {

/**
 * Sets a table of palette indices which will be searched later for a quick
 * palette match.
 * @param data the palette match table
 */

void Graphics::updatePaletteMatchTable(byte *data) {
	assert(data);
	memcpy(_paletteMatch, data, PALTABLESIZE);
}

/**
 * Matches a colour triplet to a palette index.
 * @param r red colour component
 * @param g green colour component
 * @param b blue colour component
 * @return the palette index of the closest matching colour in the palette
 */

// FIXME: This used to be inlined - probably a good idea - but the
// linker complained when I tried to use it in sprite.cpp.

uint8 Graphics::quickMatch(uint8 r, uint8 g, uint8 b) {
	return _paletteMatch[((int32) (r >> 2) << 12) + ((int32) (g >> 2) << 6) + (b >> 2)];
}

/**
 * Sets the palette.
 * @param startEntry the first colour entry to set
 * @param noEntries the number of colour entries to set
 * @param colourTable the new colour entries
 * @param fadeNow whether to perform the change immediately or delayed
 */

void Graphics::setPalette(int16 startEntry, int16 noEntries, byte *colourTable, uint8 fadeNow) {
	assert(noEntries > 0);

	memcpy(&_palette[4 * startEntry], colourTable, noEntries * 4);

	if (fadeNow == RDPAL_INSTANT) {
		_vm->_system->setPalette(_palette, startEntry, noEntries);
		setNeedFullRedraw();
	}
}

void Graphics::dimPalette(void) {
	byte *p = _palette;

	for (int i = 0; i < 256; i++) {
		p[i * 4 + 0] /= 2;
		p[i * 4 + 1] /= 2;
		p[i * 4 + 2] /= 2;
	}

	_vm->_system->setPalette(p, 0, 256);
	setNeedFullRedraw();
}

/**
 * Fades the palette up from black to the current palette.
 * @param time the time it will take the palette to fade up
 */

int32 Graphics::fadeUp(float time) {
	if (getFadeStatus() != RDFADE_BLACK && getFadeStatus() != RDFADE_NONE)
		return RDERR_FADEINCOMPLETE;

	_fadeTotalTime = (int32) (time * 1000);
	_fadeStatus = RDFADE_UP;
	_fadeStartTime = _vm->_system->getMillis();

	return RD_OK;
}

/**
 * Fades the palette down to black from the current palette.
 * @param time the time it will take the palette to fade down
 */

int32 Graphics::fadeDown(float time) {
	if (getFadeStatus() != RDFADE_BLACK && getFadeStatus() != RDFADE_NONE)
		return RDERR_FADEINCOMPLETE;

	_fadeTotalTime = (int32) (time * 1000);
	_fadeStatus = RDFADE_DOWN;
	_fadeStartTime = _vm->_system->getMillis();

	return RD_OK;
}

/**
 * Get the current fade status
 * @return RDFADE_UP (fading up), RDFADE_DOWN (fading down), RDFADE_NONE
 * (not faded), or RDFADE_BLACK (completely faded down)
 */

uint8 Graphics::getFadeStatus(void) {
	return _fadeStatus;
}

void Graphics::waitForFade(void) {
	while (getFadeStatus() != RDFADE_NONE && getFadeStatus() != RDFADE_BLACK) {
		updateDisplay();
		_vm->_system->delayMillis(20);
	}
}

void Graphics::fadeServer(void) {
	static int32 previousTime = 0;
	byte fadePalette[256 * 4];
	byte *newPalette = fadePalette;
	int32 currentTime;
	int16 fadeMultiplier;
	int16 i;

	// If we're not in the process of fading, do nothing.
	if (getFadeStatus() != RDFADE_UP && getFadeStatus() != RDFADE_DOWN)
		return;

	// I don't know if this is necessary, but let's limit how often the
	// palette is updated, just to be safe.
	currentTime = _vm->_system->getMillis();
	if (currentTime - previousTime <= 25)
		return;

	previousTime = currentTime;

	if (getFadeStatus() == RDFADE_UP) {
		if (currentTime >= _fadeStartTime + _fadeTotalTime) {
			_fadeStatus = RDFADE_NONE;
			newPalette = _palette;
		} else {
			fadeMultiplier = (int16) (((int32) (currentTime - _fadeStartTime) * 256) / _fadeTotalTime);
			for (i = 0; i < 256; i++) {
				newPalette[i * 4 + 0] = (_palette[i * 4 + 0] * fadeMultiplier) >> 8;
				newPalette[i * 4 + 1] = (_palette[i * 4 + 1] * fadeMultiplier) >> 8;
				newPalette[i * 4 + 2] = (_palette[i * 4 + 2] * fadeMultiplier) >> 8;
			}
		}
	} else {
		if (currentTime >= _fadeStartTime + _fadeTotalTime) {
			_fadeStatus = RDFADE_BLACK;
			memset(newPalette, 0, sizeof(fadePalette));
		} else {
			fadeMultiplier = (int16) (((int32) (_fadeTotalTime - (currentTime - _fadeStartTime)) * 256) / _fadeTotalTime);
			for (i = 0; i < 256; i++) {
				newPalette[i * 4 + 0] = (_palette[i * 4 + 0] * fadeMultiplier) >> 8;
				newPalette[i * 4 + 1] = (_palette[i * 4 + 1] * fadeMultiplier) >> 8;
				newPalette[i * 4 + 2] = (_palette[i * 4 + 2] * fadeMultiplier) >> 8;
			}
		}
	}

	_vm->_system->setPalette(newPalette, 0, 256);
	setNeedFullRedraw();
}

} // End of namespace Sword2
