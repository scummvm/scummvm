/* Copyright (C) 1994-2003 Revolution Software Ltd
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
// #include "common/util.h"
// #include "base/engine.h"
#include "sword2/sword2.h"

namespace Sword2 {

uint8 Graphics::getMatch(uint8 r, uint8 g, uint8 b) {
	int32 diff;
	int32 min;
	int16 diffred, diffgreen, diffblue;
	int16 i;
	uint8 minIndex;

	diffred = _palCopy[0][0] - r;
	diffgreen = _palCopy[0][1] - g;
	diffblue = _palCopy[0][2] - b;

	diff = diffred * diffred + diffgreen * diffgreen + diffblue * diffblue;
	min = diff;
	minIndex = 0;
	if (diff > 0) {
		for (i = 1; i < 256; i++) {
			diffred = _palCopy[i][0] - r;
			diffgreen = _palCopy[i][1] - g;
			diffblue = _palCopy[i][2] - b;

			diff = diffred * diffred + diffgreen * diffgreen + diffblue * diffblue;
			if (diff < min) {
				min = diff;
				minIndex = (uint8) i;
				if (min == 0)
					break;
			}
		}
	}

	// Here, minIndex is the index of the matchpalette which is closest.
	return minIndex;
}

/**
 * Sets or creates a table of palette indices which will be searched later for
 * a quick palette match.
 * @param data either the palette match table, or NULL to create a new table
 * from the current palCopy
 */

void Graphics::updatePaletteMatchTable(uint8 *data) {
	if (!data) {
		int16 red, green, blue;
		uint8 *p;

		// Create palette match table

		// FIXME: Does this case ever happen?

		p = &_paletteMatch[0];
		for (red = 0; red < 256; red += 4) {
			for (green = 0; green < 256; green += 4) {
				for (blue = 0; blue < 256; blue += 4) {
					*p++ = getMatch((uint8) red, (uint8) green, (uint8) blue);
				}
			}
		}
	} else {
		// The provided data is the new palette match table
		memcpy(_paletteMatch, data, PALTABLESIZE);
	}
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
 */

void Graphics::setPalette(int16 startEntry, int16 noEntries, uint8 *colourTable, uint8 fadeNow) {
	if (noEntries) {
		memcpy(&_palCopy[startEntry][0], colourTable, noEntries * 4);
		if (fadeNow == RDPAL_INSTANT) {
			_vm->_system->set_palette((const byte *) _palCopy, startEntry, noEntries);
			setNeedFullRedraw();
		}
	} else {
		_vm->_system->set_palette((const byte *) _palCopy, 0, 256);
		setNeedFullRedraw();
	}
}

void Graphics::dimPalette(void) {
	byte *p = (byte *) _palCopy;

	for (int i = 0; i < 256 * 4; i++)
		p[i] /= 2;

	_vm->_system->set_palette(p, 0, 256);
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
	_fadeStartTime = _vm->_system->get_msecs();

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
	_fadeStartTime = _vm->_system->get_msecs();

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
		_vm->_system->delay_msecs(20);
	}
}

void Graphics::fadeServer(void) {
	static int32 previousTime = 0;
	const byte *newPalette = (const byte *) _fadePalette;
	int32 currentTime;
	int16 fadeMultiplier;
	int16 i;

	// This used to be called through a timer, but is now called from
	// ServiceWindows() instead, since that's the only place where we
	// actually update the screen.

	// If we're not in the process of fading, do nothing.
	if (getFadeStatus() != RDFADE_UP && getFadeStatus() != RDFADE_DOWN)
		return;

	// I don't know if this is necessary, but let's limit how often the
	// palette is updated, just to be safe.
	currentTime = _vm->_system->get_msecs();
	if (currentTime - previousTime <= 25)
		return;

	previousTime = currentTime;

	if (getFadeStatus() == RDFADE_UP) {
		if (currentTime >= _fadeStartTime + _fadeTotalTime) {
			_fadeStatus = RDFADE_NONE;
			newPalette = (const byte *) _palCopy;
		} else {
			fadeMultiplier = (int16) (((int32) (currentTime - _fadeStartTime) * 256) / _fadeTotalTime);
			for (i = 0; i < 256; i++) {
				_fadePalette[i][0] = (_palCopy[i][0] * fadeMultiplier) >> 8;
				_fadePalette[i][1] = (_palCopy[i][1] * fadeMultiplier) >> 8;
				_fadePalette[i][2] = (_palCopy[i][2] * fadeMultiplier) >> 8;
			}
		}
	} else {
		if (currentTime >= _fadeStartTime + _fadeTotalTime) {
			_fadeStatus = RDFADE_BLACK;
			memset(_fadePalette, 0, sizeof(_fadePalette));
		} else {
			fadeMultiplier = (int16) (((int32) (_fadeTotalTime - (currentTime - _fadeStartTime)) * 256) / _fadeTotalTime);
			for (i = 0; i < 256; i++) {
				_fadePalette[i][0] = (_palCopy[i][0] * fadeMultiplier) >> 8;
				_fadePalette[i][1] = (_palCopy[i][1] * fadeMultiplier) >> 8;
				_fadePalette[i][2] = (_palCopy[i][2] * fadeMultiplier) >> 8;
			}
		}
	}

	_vm->_system->set_palette(newPalette, 0, 256);
	setNeedFullRedraw();
}

} // End of namespace Sword2
