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

#include "common/scummsys.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "mads/mads.h"
#include "mads/msurface.h"

namespace MADS {

RGBList::RGBList(int numEntries, byte *srcData, bool freeData) {
	_size = numEntries;
	assert(numEntries <= PALETTE_COUNT);

	if (srcData == NULL) {
		_data = new byte[numEntries * 3];
		_freeData = true;
	} else {
		_data = srcData;
		_freeData = freeData;
	}

	_palIndexes = new byte[numEntries];
	Common::fill(&_palIndexes[0], &_palIndexes[numEntries], 0);
}

RGBList::~RGBList() {
	if (_freeData)
		delete[] _data;
	delete[] _palIndexes;
}

/*------------------------------------------------------------------------*/

#define VGA_COLOR_TRANS(x) (x == 0x3f ? 255 : x << 2)

Palette::Palette(MADSEngine *vm) : _vm(vm) {
	reset();
	_fading_in_progress = false;
	Common::fill(&_usageCount[0], &_usageCount[PALETTE_COUNT], 0);
	Common::fill(&_mainPalette[0], &_mainPalette[PALETTE_SIZE], 0);
}

void Palette::setPalette(const byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->setPalette(colors, start, num);
	reset();
}

void Palette::grabPalette(byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->grabPalette(colors, start, num);
	reset();
}

uint8 Palette::palIndexFromRgb(byte r, byte g, byte b, byte *paletteData) {
	byte index = 0;
	int32 minDist = 0x7fffffff;
	byte palData[PALETTE_SIZE];
	int Rdiff, Gdiff, Bdiff;

	if (paletteData == NULL) {
		g_system->getPaletteManager()->grabPalette(palData, 0, PALETTE_COUNT);
		paletteData = &palData[0];
	}

	for (int palIndex = 0; palIndex < PALETTE_COUNT; ++palIndex) {
		Rdiff = r - paletteData[palIndex * 3];
		Gdiff = g - paletteData[palIndex * 3 + 1];
		Bdiff = b - paletteData[palIndex * 3 + 2];

		if (Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff < minDist) {
			minDist = Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff;
			index = (uint8)palIndex;
		}
	}

	return (uint8)index;
}

void Palette::reset() {
	byte palData[PALETTE_SIZE];
	g_system->getPaletteManager()->grabPalette(palData, 0, PALETTE_COUNT);

	BLACK = palIndexFromRgb(0, 0, 0, palData);
	BLUE = palIndexFromRgb(0, 0, 255, palData);
	GREEN = palIndexFromRgb(0, 255, 0, palData);
	CYAN = palIndexFromRgb(0, 255, 255, palData);
	RED = palIndexFromRgb(255, 0, 0, palData);
	VIOLET = palIndexFromRgb(255, 0, 255, palData);
	BROWN = palIndexFromRgb(168, 84, 84, palData);
	LIGHT_GRAY = palIndexFromRgb(168, 168, 168, palData);
	DARK_GRAY = palIndexFromRgb(84, 84, 84, palData);
	LIGHT_BLUE = palIndexFromRgb(0, 0, 127, palData);
	LIGHT_GREEN = palIndexFromRgb(0, 127, 0, palData);
	LIGHT_CYAN = palIndexFromRgb(0, 127, 127, palData);
	LIGHT_RED = palIndexFromRgb(84, 0, 0, palData);
	PINK = palIndexFromRgb(84, 0, 0, palData);
	YELLOW = palIndexFromRgb(0, 84, 84, palData);
	WHITE = palIndexFromRgb(255, 255, 255, palData);
}

void Palette::fadeIn(int numSteps, uint delayAmount, RGBList *destPalette) {
	fadeIn(numSteps, delayAmount, destPalette->data(), destPalette->size());
}

void Palette::fadeIn(int numSteps, uint delayAmount, byte *destPalette, int numColors) {
	if (_fading_in_progress)
		return;

	_fading_in_progress = true;
	byte blackPalette[PALETTE_SIZE];
	Common::fill(&blackPalette[0], &blackPalette[PALETTE_SIZE], 0);

	// Initially set the black palette
	_vm->_palette->setPalette(blackPalette, 0, numColors);

	// Handle the actual fading
	fadeRange(blackPalette, destPalette, 0, numColors - 1, numSteps, delayAmount);

	_fading_in_progress = false;
}

void Palette::resetColorCounts() {
	Common::fill(&_usageCount[0], &_usageCount[PALETTE_COUNT], 0);
}

void Palette::blockRange(int startIndex, int size) {
	// Use a reference count of -1 to signal a palette index shouldn't be used
	Common::fill(&_usageCount[startIndex], &_usageCount[startIndex + size], -1);
}

void Palette::addRange(RGBList *list) {
	byte *data = list->data();
	byte *palIndexes = list->palIndexes();
	byte palData[PALETTE_COUNT];
	g_system->getPaletteManager()->grabPalette(palData, 0, PALETTE_COUNT);
	bool paletteChanged = false;
	
	for (int colIndex = 0; colIndex < list->size(); ++colIndex) {
		// Scan through for an existing copy of the RGB value
		int palIndex = -1; 
		while (++palIndex < PALETTE_COUNT) {
			if (_usageCount[palIndex] <= 0)
				// Palette index is to be skipped
				continue;

			if ((palData[palIndex * 3] == data[colIndex * 3]) && 
				(palData[palIndex * 3 + 1] == data[colIndex * 3 + 1]) &&
				(palData[palIndex * 3 + 2] == data[colIndex * 3 + 2])) 
				// Match found
				break;
		}

		if (palIndex == PALETTE_COUNT) {
			// No match found, so find a free slot to use
			palIndex = -1;
			while (++palIndex < PALETTE_COUNT) {
				if (_usageCount[palIndex] == 0)
					break;
			}

			if (palIndex == PALETTE_COUNT) 
				error("addRange - Ran out of palette space to allocate");

			palData[palIndex * 3] = data[colIndex * 3];
			palData[palIndex * 3 + 1] = data[colIndex * 3 + 1];
			palData[palIndex * 3 + 2] = data[colIndex * 3 + 2];
			paletteChanged = true;
		}

		palIndexes[colIndex] = palIndex;
		++_usageCount[palIndex];
	}

	if (paletteChanged) {
		g_system->getPaletteManager()->setPalette(&palData[0], 0, 256);
		reset();
	}
}

void Palette::deleteRange(RGBList *list) {
	// Release the reference count on each of the palette entries
	for (int colIndex = 0; colIndex < list->size(); ++colIndex) {
		int palIndex = list->palIndexes()[colIndex];
		assert(_usageCount[palIndex] > 0);
		--_usageCount[palIndex];
	}
}

void Palette::deleteAllRanges() {
	for (int colIndex = 0; colIndex < 255; ++colIndex)
		_usageCount[colIndex] = 0;
}

void Palette::fadeRange(byte *srcPal, byte *destPal,  int startIndex, int endIndex, 
					 int numSteps, uint delayAmount) {
	byte tempPal[256 * 3];

	// perform the fade
	for(int stepCtr = 1; stepCtr <= numSteps; ++stepCtr) {
		// Delay the specified amount
		uint32 startTime = g_system->getMillis();
		while ((g_system->getMillis() - startTime) < delayAmount) {
			_vm->_events->handleEvents();
			g_system->delayMillis(10);
		}

		for (int i = startIndex; i <= endIndex; ++i) {
			// Handle the intermediate rgb values for fading
			tempPal[i * 3] = (byte) (srcPal[i * 3] + (destPal[i * 3] - srcPal[i * 3]) * stepCtr / numSteps);   
			tempPal[i * 3 + 1] = (byte) (srcPal[i * 3 + 1] + (destPal[i * 3 + 1] - srcPal[i * 3 + 1]) * stepCtr / numSteps); 
			tempPal[i * 3 + 2] = (byte) (srcPal[i * 3 + 2] + (destPal[i * 3 + 2] - srcPal[i * 3 + 2]) * stepCtr / numSteps); 
		}
		
		_vm->_palette->setPalette(&tempPal[startIndex * 3], startIndex, endIndex - startIndex + 1);
	}

	// Make sure the end palette exactly matches what is wanted
	_vm->_palette->setPalette(&destPal[startIndex * 3], startIndex, endIndex - startIndex + 1);
}

void Palette::setGradient(byte *palette, int start, int count, int rgbValue1, int rgbValue2) {
	int rgbCtr = 0;
	int rgbDiff = -(rgbValue2 - rgbValue1);
	int rgbCurrent = rgbValue2;

	if (count >  0) {
		byte *pDest = palette + start * 3;
		int endVal = count - 1;
		int numLeft = count;

		do {
			pDest[0] = pDest[1] = pDest[2] = rgbCurrent;

			if (count > 1) {
				rgbCtr += rgbDiff;
				if (rgbCtr >= endVal) {
					do {
						++rgbCurrent;
						rgbCtr += 1 - numLeft;
					} while (rgbCtr >= endVal);
				}
			}

			pDest += 3;
		} while (--numLeft > 0);
	}
}

byte *Palette::decodePalette(Common::SeekableReadStream *palStream, int *numColors) {
	*numColors = palStream->readUint16LE();
	assert(*numColors <= 252);

	byte *palData = new byte[*numColors * 3];
	Common::fill(&palData[0], &palData[*numColors * 3], 0);

	for (int i = 0; i < *numColors; ++i) {
		byte r = palStream->readByte();
		byte g = palStream->readByte();
		byte b = palStream->readByte();
		palData[i * 3] = VGA_COLOR_TRANS(r);
		palData[i * 3 + 1] = VGA_COLOR_TRANS(g);
		palData[i * 3 + 2] = VGA_COLOR_TRANS(b);

		// The next 3 bytes are unused
		palStream->skip(3);
	}

	return palData;
}

int Palette::loadPalette(Common::SeekableReadStream *palStream, int indexStart) {
	int colorCount;
	byte *palData = decodePalette(palStream, &colorCount);
	_vm->_palette->setPalette(palData, indexStart, colorCount);

	delete palData;
	return colorCount;
}

void Palette::setSystemPalette() {
	resetColorCounts();

	byte palData[4 * 3];
	palData[0 * 3] = palData[0 * 3 + 1] = palData[0 * 3 + 2] = 0;
	palData[1 * 3] = palData[1 * 3 + 1] = palData[1 * 3 + 2] = 0x54;
	palData[2 * 3] = palData[2 * 3 + 1] = palData[2 * 3 + 2] = 0xb4;
	palData[3 * 3] = palData[3 * 3 + 1] = palData[3 * 3 + 2] = 0xff;
	
	setPalette(palData, 0, 4);
	blockRange(0, 4);
}

} // End of namespace MADS
