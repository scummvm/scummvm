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

RGBList::RGBList(int numEntries, RGB8 *srcData, bool freeData) {
	_size = numEntries;
	assert(numEntries <= 256);

	if (srcData == NULL) {
		_data = new RGB8[numEntries];
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
	Common::fill(&_usageCount[0], &_usageCount[256], 0);
}

void Palette::setPalette(const byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->setPalette(colors, start, num);
	reset();
}

void Palette::setPalette(const RGB8 *colors, uint start, uint num) {
	g_system->getPaletteManager()->setPalette((const byte *)colors, start, num);
	reset();
}

void Palette::grabPalette(byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->grabPalette(colors, start, num);
	reset();
}

uint8 Palette::palIndexFromRgb(byte r, byte g, byte b, RGB8 *paletteData) {
	byte index = 0;
	int32 minDist = 0x7fffffff;
	RGB8 palData[256];
	int Rdiff, Gdiff, Bdiff;

	if (paletteData == NULL) {
		g_system->getPaletteManager()->grabPalette((byte *)palData, 0, 256);
		paletteData = &palData[0];
	}

	for (int palIndex = 0; palIndex < 256; ++palIndex) {
		Rdiff = r - paletteData[palIndex].r;
		Gdiff = g - paletteData[palIndex].g;
		Bdiff = b - paletteData[palIndex].b;

		if (Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff < minDist) {
			minDist = Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff;
			index = (uint8)palIndex;
		}
	}

	return (uint8)index;
}

void Palette::reset() {
	RGB8 palData[256];
	g_system->getPaletteManager()->grabPalette((byte *)palData, 0, 256);

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

void Palette::fadeIn(int numSteps, uint delayAmount, RGB8 *destPalette, int numColors) {
	if (_fading_in_progress)
		return;

	_fading_in_progress = true;
	RGB8 blackPalette[256];
	Common::fill((byte *)&blackPalette[0], (byte *)&blackPalette[256], 0);

	// Initially set the black palette
	_vm->_palette->setPalette(blackPalette, 0, numColors);

	// Handle the actual fading
	fadeRange(blackPalette, destPalette, 0, numColors - 1, numSteps, delayAmount);

	_fading_in_progress = false;
}

RGB8 *Palette::decodeMadsPalette(Common::SeekableReadStream *palStream, int *numColors) {
	*numColors = palStream->readUint16LE();
	assert(*numColors <= 252);

	RGB8 *palData = new RGB8[*numColors];
	Common::fill((byte *)&palData[0], (byte *)&palData[*numColors], 0);

	for (int i = 0; i < *numColors; ++i) {
		byte r = palStream->readByte();
		byte g = palStream->readByte();
		byte b = palStream->readByte();
		palData[i].r = VGA_COLOR_TRANS(r);
		palData[i].g = VGA_COLOR_TRANS(g);
		palData[i].b = VGA_COLOR_TRANS(b);

		// The next 3 bytes are unused
		palStream->skip(3);
	}

	return palData;
}

int Palette::setMadsPalette(Common::SeekableReadStream *palStream, int indexStart) {
	int colorCount;
	RGB8 *palData = Palette::decodeMadsPalette(palStream, &colorCount);
	_vm->_palette->setPalette(palData, indexStart, colorCount);
	delete palData;
	return colorCount;
}

void Palette::setMadsSystemPalette() {
	// Rex Nebular default system palette
	resetColorCounts();

	RGB8 palData[4];
	palData[0].r = palData[0].g = palData[0].b = 0;
	palData[1].r = palData[1].g = palData[1].b = 0x54;
	palData[2].r = palData[2].g = palData[2].b = 0xb4;
	palData[3].r = palData[3].g = palData[3].b = 0xff;
	
	setPalette(palData, 0, 4);
	blockRange(0, 4);
}

void Palette::resetColorCounts() {
	Common::fill(&_usageCount[0], &_usageCount[256], 0);
}

void Palette::blockRange(int startIndex, int size) {
	// Use a reference count of -1 to signal a palette index shouldn't be used
	Common::fill(&_usageCount[startIndex], &_usageCount[startIndex + size], -1);
}

void Palette::addRange(RGBList *list) {
	RGB8 *data = list->data();
	byte *palIndexes = list->palIndexes();
	RGB8 palData[256];
	g_system->getPaletteManager()->grabPalette((byte *)&palData[0], 0, 256);
	bool paletteChanged = false;
	
	for (int colIndex = 0; colIndex < list->size(); ++colIndex) {
		// Scan through for an existing copy of the RGB value
		int palIndex = -1; 
		while (++palIndex < 256) {
			if (_usageCount[palIndex] <= 0)
				// Palette index is to be skipped
				continue;

			if ((palData[palIndex].r == data[colIndex].r) && 
				(palData[palIndex].g == data[colIndex].g) &&
				(palData[palIndex].b == data[colIndex].b)) 
				// Match found
				break;
		}

		if (palIndex == 256) {
			// No match found, so find a free slot to use
			palIndex = -1;
			while (++palIndex < 256) {
				if (_usageCount[palIndex] == 0)
					break;
			}

			if (palIndex == 256) 
				error("addRange - Ran out of palette space to allocate");

			palData[palIndex].r = data[colIndex].r;
			palData[palIndex].g = data[colIndex].g;
			palData[palIndex].b = data[colIndex].b;
			paletteChanged = true;
		}

		palIndexes[colIndex] = palIndex;
		++_usageCount[palIndex];
	}

	if (paletteChanged) {
		g_system->getPaletteManager()->setPalette((byte *)&palData[0], 0, 256);
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

void Palette::fadeRange(RGB8 *srcPal, RGB8 *destPal,  int startIndex, int endIndex, 
					 int numSteps, uint delayAmount) {
	RGB8 tempPal[256];

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
			tempPal[i].r = (byte) (srcPal[i].r + (destPal[i].r - srcPal[i].r) * stepCtr / numSteps);   
			tempPal[i].g = (byte) (srcPal[i].g + (destPal[i].g - srcPal[i].g) * stepCtr / numSteps); 
			tempPal[i].b = (byte) (srcPal[i].b + (destPal[i].b - srcPal[i].b) * stepCtr / numSteps); 
		}
		
		_vm->_palette->setPalette(&tempPal[startIndex], startIndex, endIndex - startIndex + 1);
	}

	// Make sure the end palette exactly matches what is wanted
	_vm->_palette->setPalette(&destPal[startIndex], startIndex, endIndex - startIndex + 1);
}

} // End of namespace MADS
