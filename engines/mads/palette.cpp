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

void RGB6::load(Common::SeekableReadStream *f) {
	r = f->readByte();
	g = f->readByte();
	b = f->readByte();
	palIndex = f->readByte();
	u2 = f->readByte();
	flags = f->readByte();
}

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

PaletteUsage::PaletteUsage() {
}

void PaletteUsage::load(int count, ...) {
	va_list va;
	va_start(va, count);

	_data.clear();
	for (int i = 0; i < count; ++i)
		_data.push_back(va_arg(va, int));

	va_end(va);
}


void PaletteUsage::getKeyEntries(Common::Array<RGB6> &palette) {
	_data.clear();

	 for (uint i = 0; i < palette.size(); ++i) {
		 byte *uPtr = &palette[i].flags;
		 if ((*uPtr & 0x10) && _data.size() < 3) {
			 _data.push_back(i);
		 }
	 }
}

void PaletteUsage::prioritize(Common::Array<RGB6> &palette) {
	int lst[3];

	for (uint i = 0; i < _data.size(); ++i) {
		RGB6 &palEntry = palette[_data[i]];
		lst[i] = rgbMerge(palEntry);
	}
	
	prioritizeFromList(lst);
}

int PaletteUsage::rgbMerge(RGB6 &palEntry) {
	return palEntry.r * 38 + palEntry.g * 76 + palEntry.b * 14;
}

void PaletteUsage::prioritizeFromList(int lst[3]) {
	int idx1 = _data.size() - 1;
	bool continueFlag;
	int count2;

	do {
		continueFlag = false;
		count2 = 0;

		if (idx1 > 0) {
			int numEntries = _data.size() - 1;
			int usageIndex = 0, lstIndex = 0;

			do {
				if (lst[lstIndex] < lst[lstIndex + 1]) {
					int lstVal = lst[lstIndex];
					int usageVal = _data[usageIndex];

					if (numEntries > 0) {
						Common::copy(&lst[lstIndex + 1], &lst[lstIndex + numEntries], &lst[lstIndex]);
						_data.remove_at(usageIndex);
						_data.push_back(0);
					}
					
					int newIdx = 0;
					if (idx1 > 0 && !continueFlag) {
						for (newIdx = 0; newIdx <= idx1; ++newIdx) {
							if (lst[newIdx] > lstVal)
								break;
						}
					}

					continueFlag = true;
					int idxDiff = _data.size() - newIdx - 1;
					if (idxDiff > 0) {
						Common::copy_backward(&lst[0], &lst[2], &lst[1]);
						_data.remove_at(2);
						_data.insert_at(0, 0);
					}

					lst[newIdx] = lstVal;
					_data[newIdx] = usageVal;
				}

				++usageIndex;
				--numEntries;
				++lstIndex;
				++count2;
			} while (count2 > idx1 && !continueFlag);
		}
	} while (continueFlag);
}

void PaletteUsage::transform(Common::Array<RGB6> &palette) {
	if (!empty()) {
		for (uint i = 0; i < _data.size(); ++i) {
			int palIndex = _data[i];
			_data[i] = palette[palIndex].palIndex;
		}
	}
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

void Palette::setGradient(byte *palette, int start, int count, int rgbValue1, int rgbValue2) {
	int rgbCtr = 0;
	int rgbCurrent = rgbValue2;
	int rgbDiff = -(rgbValue2 - rgbValue1);

	if (count >  0) {
		byte *pDest = palette + start * 3;
		int endVal = count - 1;
		int numLeft = count;

		do {
			pDest[0] = pDest[1] = pDest[2] = rgbCurrent;

			if (numLeft > 1) {
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

void Palette::resetGamePalette(int lowRange, int highRange) {
	Common::fill((byte *)&_gamePalette[0], (byte *)&_gamePalette[PALETTE_COUNT], 0);
	initRange(_mainPalette);

	// Init low range to common RGB values
	if (lowRange) {
		_gamePalette[0].r = 1;
		_gamePalette[0].b = 0;

		Common::fill(&_gamePalette[1], &_gamePalette[lowRange - 1], _gamePalette[0]);
	}

	// Init high range to common RGB values
	if (highRange) {
		_gamePalette[255].r = 1;
		_gamePalette[255].b = 0;

		Common::fill(&_gamePalette[255 - highRange], &_gamePalette[254], _gamePalette[255]);
	}
}

void Palette::initGamePalette() {
	// TODO
}

void Palette::initRange(byte *palette) {
	int var6 = 0;
	int vdx = 0;
	int vbx = 0;
	do {
		int vdi = (vdx == 1) ? 0 : 0x2A;
		int var8 = 0;
		int varE = vbx;
		int var10 = vdx;
		do {
			vdx = 0;
			do {
				int vcx = 0;
				int var4 = vdx;
				do {
					int var2 = var6 + vcx;
					byte *destP = &palette[var2 * 3];
					
					destP[0] = (var8) ? vdi & 0xFF : vbx & 0XFF;
					destP[1] = (var4) ? vdi & 0xFF : vbx & 0XFF;
					destP[2] = (vcx) ? vdi & 0xFF : vbx & 0XFF;					
				} while (++vcx < 2);

				var6 += 2;
				vdx = var4;
			} while (++vdx < 2);
		} while (++var8 < 2);
	
		vdx = var10 + 1;
		vbx = varE + 21;
	} while (vbx < 42);

	palette[19] = 21;
}

void Palette::setLowRange() {
	_mainPalette[0] = _mainPalette[1] = _mainPalette[2] = 0; 
	_mainPalette[3] = _mainPalette[4] = _mainPalette[5] = 0x15; 
	_mainPalette[6] = _mainPalette[7] = _mainPalette[8] = 0x2A; 
	_mainPalette[9] = _mainPalette[10] = _mainPalette[11] = 0x3F; 
	_vm->_palette->setPalette(_mainPalette, 0, 4);
}

} // End of namespace MADS
