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

int PaletteUsage::process(Common::Array<RGB6> &palette, int v) {
	byte *palette1 = nullptr, *palette2 = nullptr;
	int palLow;
	int palHigh = (v & 0x800) ? 0x100 : 0xC;
	int palIdx;
	assert(v >= 0);

	if (v & 0x4000) {
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
	int varA = v & 0x8000;
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
	byte *pal1 = new byte[PALETTE_COUNT];
	byte *pal2 = new byte[PALETTE_COUNT];

	for (uint palIndex = 0; palIndex < palette.size(); ++palIndex) {
		pal2[palIndex] = palIndex;
		pal1[palIndex] = 0;

		if (!(palette[palIndex]._flags & 0x80)) {
			pal1[palIndex] = 0x40;
		}
		if (!(palette[palIndex]._flags & 0x60)) {
			pal1[palIndex] |= 0x20;
		}
	}

	_vm->_palette->processLists(palette.size(), pal1, pal2);
	
	int var3A = (v & 0x4000) ? 0xffff : 0xfffe;

	for (uint palIndex = 0; palIndex < palette.size(); ++palIndex) {
		bool var48 = false;
		int var4 = 0xffff;
		int v1 = pal1[palIndex];

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
				((v & 0x2000) && !(palette[palIndex]._flags & 0x40)) ||
				((v & 0x1000) && (palCount > 0))
				) ? 1 : 0x7fff;
			int var36 = (palette[palIndex]._flags & 0x80) ? 0 : 2;
			
			for (int idx = palLow; idx < palIdx; ++idx) {
				// TODO
			}
		}
		//TODO
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
		RGB4 &r = _vm->_palette->_gamePalette[i];
		if (!(r.r | r.g | r.b | r.u)) {
			++count;
			if (*palIndex < 0)
				*palIndex = i;
		}
	}

	return count;
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

#define VGA_COLOR_TRANS(x) (x == 0x3f ? 255 : x << 2)

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
	byte palData[4 * 3];
	palData[0 * 3] = palData[0 * 3 + 1] = palData[0 * 3 + 2] = 0;
	palData[1 * 3] = palData[1 * 3 + 1] = palData[1 * 3 + 2] = 0x54;
	palData[2 * 3] = palData[2 * 3 + 1] = palData[2 * 3 + 2] = 0xb4;
	palData[3 * 3] = palData[3 * 3 + 1] = palData[3 * 3 + 2] = 0xff;
	
	setPalette(palData, 0, 4);
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

	_rgbList.clear();
	_v1 = 0;
	_lowRange = lowRange;
	_highRange = highRange;
}

void Palette::initGamePalette() {
	RGB4 rgb;
	rgb.r = 1;

	if (_vm->_game->_player._spritesLoaded && _vm->_game->_player._numSprites) {

		for (int idx = 0; idx < _vm->_game->_player._numSprites; ++idx) {
			SpriteAsset *asset = _vm->_game->_scene._sprites[
				_vm->_game->_player._spritesStart + idx];
			
			uint32 mask = 1;
			if (asset->_field6)
				mask <<= asset->_field6;
			
			rgb.r = mask & 0xff;
			rgb.g = (mask >> 8) & 0xff;
			rgb.b = (mask >> 16) & 0xff;
			rgb.u = (mask >> 24) & 0xff;
		}
	}

	for (int idx = 0; idx < PALETTE_COUNT; ++idx) {
		_gamePalette[idx].r &= rgb.r;
		_gamePalette[idx].g &= rgb.g;
		_gamePalette[idx].b &= rgb.b;
		_gamePalette[idx].u &= rgb.u;
	}

	_v1 = 0;
	_rgbList.reset();
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

void Palette::fadeOut(byte palette[PALETTE_SIZE], int v1, int v2, int v3, int v4, int v5, int v6) {

}


} // End of namespace MADS
