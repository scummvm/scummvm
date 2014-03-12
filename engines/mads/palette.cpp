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

#define VGA_COLOR_TRANS(x) (((((int)(x)) + 1) << 2) - 1)

void RGB6::load(Common::SeekableReadStream *f) {
	r = VGA_COLOR_TRANS(f->readByte());
	g = VGA_COLOR_TRANS(f->readByte());
	b = VGA_COLOR_TRANS(f->readByte());
	_palIndex = f->readByte();
	_u2 = f->readByte();
	_flags = f->readByte();
}

/*------------------------------------------------------------------------*/

PaletteUsage::PaletteUsage(MADSEngine *vm) {
	_vm = vm;
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
		 byte *uPtr = &palette[i]._flags;
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

int PaletteUsage::process(Common::Array<RGB6> &palette, uint flags) {
	byte *pal1 = nullptr, *pal2 = nullptr;
	int palLow;
	int palHigh = (flags & 0x800) ? 0x100 : 0xFC;
	int palIdx;

	if (flags & 0x4000) {
		palLow = 0;
		palIdx = palHigh;
	} else {
		palLow = _vm->_palette->_lowRange;
		if ((PALETTE_COUNT - _vm->_palette->_highRange) > palHigh) {
			palIdx = palHigh;
		} else {
			palIdx = PALETTE_COUNT - _vm->_palette->_highRange;
		}
	}

	int rgbIndex = _vm->_palette->_rgbList.scan();
	uint32 rgbMask = 1 << rgbIndex;
	int varA = flags & 0x8000;
	bool hasUsage = !_vm->_palette->_paletteUsage.empty();
	bool flag1 = false;

	if (hasUsage) {
		if (varA || !_vm->_palette->_paletteUsage[0])
			hasUsage = false;

		if (varA && _vm->_palette->_paletteUsage[0])
			flag1 = true;
	}

	if (hasUsage) {
		getKeyEntries(palette);
		prioritize(palette);
	}

	int freeIndex;
	int palCount = getGamePalFreeIndex(&freeIndex);
	pal1 = new byte[PALETTE_COUNT];
	pal2 = new byte[PALETTE_COUNT];

	for (uint palIndex = 0; palIndex < palette.size(); ++palIndex) {
		pal2[palIndex] = palIndex;
		pal1[palIndex] = 0;

		if (!(palette[palIndex]._flags & 0x80)) {
			pal1[palIndex] = 0x40;
		}
		if (palette[palIndex]._flags & 0x60) {
			pal1[palIndex] |= 0x20;
		}
	}

	_vm->_palette->processLists(palette.size(), pal1, pal2);
	
	int var3A = (flags & 0x4000) ? 0xffff : 0xfffe;

	for (uint palIndex = 0; palIndex < palette.size(); ++palIndex) {
		bool var48 = false;
		int var4 = 0xffff;
		int v1 = pal2[palIndex];

		if (palette[v1]._flags & 8) {
			var48 = true;
			var4 = 0xFD;
		}

		if (hasUsage && palette[v1]._flags & 0x10) {
			for (uint usageIndex = 0; usageIndex < _data.size() && !var48; ++usageIndex) {
				if (_data[usageIndex] == palIndex) {
					var48 = true;
					int dataIndex = MIN(usageIndex, _data.size() - 1);
					var4 = _data[dataIndex];
				}
			}
		}

		if (flag1 && palette[palIndex]._flags & 0x10) {
			for (uint usageIndex = 0; usageIndex < _data.size() && !var48; ++usageIndex) {
				if (_data[usageIndex] == palIndex) {
					var48 = true;
					var4 = 0xF0 + usageIndex;

					// Copy data into the high end of the main palette
					RGB6 &pSrc = palette[palIndex];
					byte *pDest = &_vm->_palette->_mainPalette[var4 * 3];
					pDest[0] = pSrc.r;
					pDest[1] = pSrc.g;
					pDest[2] = pSrc.b;
				}
			}
		}

		if (!var48 && !varA) {
			int var2 = !(palette[palIndex]._flags & 0x20) && (
				((flags & 0x2000) && !(palette[palIndex]._flags & 0x40)) ||
				((flags & 0x1000) && (palCount > 0))
				) ? 1 : 0x7fff;
			int var36 = (palette[palIndex]._flags & 0x80) ? 0 : 2;
			
			for (int idx = palLow; idx < palIdx; ++idx) {
				uint32 v = _vm->_palette->_palFlags[idx];
				if ((v & var3A) && !(v & var36)) {
					int var10;

					if (var2 > 1) {
						var10 = rgbFactor(&_vm->_palette->_mainPalette[idx * 3], palette[palIndex]);
					}
					else if (_vm->_palette->_mainPalette[idx * 3] != palette[palIndex].r ||
							_vm->_palette->_mainPalette[idx * 3 + 1] != palette[palIndex].g ||
							_vm->_palette->_mainPalette[idx * 3 + 2] != palette[palIndex].b) {
						var10 = 1;
					} else {
						var10 = 0;
					}

					if (var2 > var10) {
						var48 = true;
						var4 = idx;
						var2 = var10;
					}
				}
			}
		}

		if (!var48 && (!(flags & 0x1000) || (!(palette[palIndex]._flags & 0x60) && !(flags & 0x2000)))) {
			for (int idx = freeIndex; idx < palIdx && !var48; ++idx) {
				if (!_vm->_palette->_palFlags[idx]) {
					--palCount;
					++freeIndex;
					var48 = true;
					var4 = idx;

					RGB6 &pSrc = palette[palIndex];
					byte *pDest = &_vm->_palette->_mainPalette[idx * 3];
					pDest[0] = pSrc.r;
					pDest[1] = pSrc.g;
					pDest[2] = pSrc.b;
				}
			}
		}
		
		assert(var48);
		int var52 = (varA && palette[palIndex]._u2) ? 2 : 0;

		_vm->_palette->_palFlags[var4] |= var52 | rgbMask;
		palette[palIndex]._palIndex = var4;
	}

	_vm->_palette->_rgbList[rgbIndex] = 0xffff;

	delete[] pal1;
	delete[] pal2;

	return rgbIndex;
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
			_data[i] = palette[palIndex]._palIndex;
		}
	}
}

int PaletteUsage::getGamePalFreeIndex(int *palIndex) {
	*palIndex = -1;
	int count = 0;

	for (int i = 0; i < PALETTE_COUNT; ++i) {
		if (!_vm->_palette->_palFlags[i]) {
			++count;
			if (*palIndex < 0)
				*palIndex = i;
		}
	}

	return count;
}

int PaletteUsage::rgbFactor(byte *palEntry, RGB6 &pal6) {
	int total = 0;
	total += (palEntry[0] - pal6.r) * (palEntry[0] - pal6.r);
	total += (palEntry[1] - pal6.g) * (palEntry[1] - pal6.g);
	total += (palEntry[2] - pal6.b) * (palEntry[2] - pal6.b);

	return total;
}

/*------------------------------------------------------------------------*/

void RGBList::clear() {
	Common::fill(&_data[0], &_data[32], 0);
}

void RGBList::reset() {
	Common::fill(&_data[2], &_data[32], 0);
}

int RGBList::scan() {
	for (int i = 0; i < 32; ++i) {
		if (!_data[i])
			return i;
	}

	error("List was full");
}

/*------------------------------------------------------------------------*/

Palette::Palette(MADSEngine *vm) : _vm(vm), _paletteUsage(vm) {
	reset();

	_v1 = 0;
	_lowRange = 0;
	_highRange = 0;
	Common::fill(&_mainPalette[0], &_mainPalette[PALETTE_SIZE], 0);
}

void Palette::setPalette(const byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->setPalette(colors, start, num);
	reset();
}

void Palette::setEntry(byte palIndex, byte r, byte g, byte b) {
	_mainPalette[palIndex * 3] = r;
	_mainPalette[palIndex * 3 + 1] = g;
	_mainPalette[palIndex * 3 + 2] = b;

	setPalette((const byte *)&_mainPalette[palIndex * 3], palIndex, 1);
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

void Palette::processLists(int count, byte *pal1, byte *pal2) {
	bool continueFlag;
	int endIndex = count - 1;

	do {
		continueFlag = false;

		for (int palIndex = 0; palIndex < endIndex && !continueFlag; ++palIndex) {
			byte *palP = &pal1[palIndex];
			byte *pal2P = &pal2[palIndex];

			if (palP[1] < palP[0]) {
				int v1 = palP[0];
				int v2 = pal2P[0];

				int size = count - palIndex - 1;
				if (size > 0) {
					Common::copy(palP + 1, palP + size + 1, palP);
					Common::copy(pal2P + 1, pal2P + size + 1, pal2P);
				}

				int idx;
				for (idx = 0; idx < endIndex && !continueFlag; ++idx) {
					if (pal1[idx] > v1)
						continueFlag = true;
				}
				continueFlag = true;

				int size2 = count - idx - 1;
				if (size2 > 0) {
					Common::copy(palP + idx, palP + idx + size2 + 1, palP);
					Common::copy(pal2P + idx, pal2P + idx + size2 + 1, pal2P);
				}

				pal1[idx] = v1;
				pal2[idx] = v2;
			}
		}

	} while (continueFlag);
}


void Palette::decodePalette(Common::SeekableReadStream *palStream, uint flags) {
	int numColors = palStream->readUint16LE();
	assert(numColors <= 252);

	// Load in the palette
	Common::Array<RGB6> palette;
	palette.resize(numColors);
	for (int i = 0; i < numColors; ++i)
		palette[i].load(palStream);
	
	// Process the palette data
	_paletteUsage.process(palette, flags);
}

void Palette::setSystemPalette() {
	byte palData[4 * 3];
	palData[0 * 3] = palData[0 * 3 + 1] = palData[0 * 3 + 2] = 0;
	palData[1 * 3] = palData[1 * 3 + 1] = palData[1 * 3 + 2] = 0x54;
	palData[2 * 3] = palData[2 * 3 + 1] = palData[2 * 3 + 2] = 0xb4;
	palData[3 * 3] = palData[3 * 3 + 1] = palData[3 * 3 + 2] = 0xff;
	
	setPalette(palData, 0, 4);
}

void Palette::resetGamePalette(int lowRange, int highRange) {
	Common::fill((byte *)&_palFlags[0], (byte *)&_palFlags[PALETTE_COUNT], 0);
	initVGAPalette(_mainPalette);

	// Init low range to common RGB values
	if (lowRange) {
		Common::fill(&_palFlags[0], &_palFlags[lowRange], 1);
	}

	// Init high range to common RGB values
	if (highRange) {
		_palFlags[255] = 1;

		Common::fill(&_palFlags[255 - highRange], &_palFlags[254], _palFlags[255]);
	}

	_rgbList.clear();
	_rgbList[0] = _rgbList[1] = 0xffff;

	_v1 = 0;
	_lowRange = lowRange;
	_highRange = highRange;
}

void Palette::initPalette() {
	RGB4 rgb;
	uint32 palMask = 1;

	if (_vm->_game->_player._spritesLoaded && _vm->_game->_player._numSprites) {

		for (int idx = 0; idx < _vm->_game->_player._numSprites; ++idx) {
			SpriteAsset *asset = _vm->_game->_scene._sprites[
				_vm->_game->_player._spritesStart + idx];
			
			uint32 mask = 1;
			if (asset->_usageIndex)
				mask <<= asset->_usageIndex;
			
			palMask = mask;
		}
	}

	for (int idx = 0; idx < PALETTE_COUNT; ++idx)
		_palFlags[idx] = palMask;

	_v1 = 0;
	_rgbList.reset();
}

void Palette::initVGAPalette(byte *palette) {
	int var6 = 0;
	int vdx = 0;
	int vbx = 0;
	byte *destP = palette;

	do {
		int vdi = (vdx == 1) ? 0x3F : 0x2A;
		int var8 = 0;
		int varE = vbx;
		int var10 = vdx;
		do {
			vdx = 0;
			do {
				int vcx = 0;
				int var4 = vdx;
				do {
					*destP++ = VGA_COLOR_TRANS((var8) ? vdi & 0xFF : vbx & 0XFF);
					*destP++ = VGA_COLOR_TRANS((var4) ? vdi & 0xFF : vbx & 0XFF);
					*destP++ = VGA_COLOR_TRANS((vcx) ? vdi & 0xFF : vbx & 0XFF);
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

void Palette::fadeOut(byte palette[PALETTE_SIZE], int v1, int v2, int v3, int v4, int v5, int v6) {

}


} // End of namespace MADS
