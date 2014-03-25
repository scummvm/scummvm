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
		_data.push_back(UsageEntry(va_arg(va, int)));

	va_end(va);
}


void PaletteUsage::getKeyEntries(Common::Array<RGB6> &palette) {
	_data.clear();

	 for (uint i = 0; i < palette.size(); ++i) {
		 byte *uPtr = &palette[i]._flags;
		 if ((*uPtr & 0x10) && _data.size() < 3) {
			 _data.push_back(UsageEntry(i));
		 }
	 }
}

static bool sortHelper(const PaletteUsage::UsageEntry &ue1, const PaletteUsage::UsageEntry &ue2) {
	return ue1._sortValue < ue2._sortValue;
}

void PaletteUsage::prioritize(Common::Array<RGB6> &palette) {
	for (uint i = 0; i < _data.size(); ++i) {
		RGB6 &palEntry = palette[_data[i]._palIndex];
		_data[i]._sortValue = rgbMerge(palEntry);
	}
	
	Common::sort(_data.begin(), _data.end(), sortHelper);
}

static bool rangeSorter(const PaletteUsage::UsageRange &ur1, const PaletteUsage::UsageRange &ur2) {
	return ur1._v2 < ur2._v2;
}

int PaletteUsage::process(Common::Array<RGB6> &palette, uint flags) {
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
		if (varA || _vm->_palette->_paletteUsage.empty())
			hasUsage = false;

		if (varA && !_vm->_palette->_paletteUsage.empty())
			flag1 = true;
	}

	if (hasUsage) {
		getKeyEntries(palette);
		prioritize(palette);
	}

	int freeIndex;
	int palCount = getGamePalFreeIndex(&freeIndex);
	Common::Array<UsageRange> palRange;

	for (uint palIndex = 0; palIndex < palette.size(); ++palIndex) {
		byte pal2 = palIndex;
		byte pal1 = 0;

		if (!(palette[palIndex]._flags & 0x80)) {
			pal1 = 0x40;
		}
		if (palette[palIndex]._flags & 0x60) {
			pal1 |= 0x20;
		}

		palRange.push_back(UsageRange(pal1, pal2));
	}

	Common::sort(palRange.begin(), palRange.end(), rangeSorter);
	
	int var3A = (flags & 0x4000) ? 0xffff : 0xfffe;

	for (uint palIndex = 0; palIndex < palette.size(); ++palIndex) {
		bool var48 = false;
		int var4 = 0xffff;
		int v1 = palRange[palIndex]._v2;

		if (palette[v1]._flags & 8) {
			var48 = true;
			var4 = 0xFD;
		}

		if (hasUsage && palette[v1]._flags & 0x10) {
			for (uint usageIndex = 0; usageIndex < _data.size() && !var48; ++usageIndex) {
				if (_data[usageIndex]._palIndex == palIndex) {
					var48 = true;
					int dataIndex = MIN(usageIndex, _data.size() - 1);
					var4 = _data[dataIndex]._palIndex;
				}
			}
		}

		if (flag1 && palette[palIndex]._flags & 0x10) {
			for (uint usageIndex = 0; usageIndex < _data.size() && !var48; ++usageIndex) {
				if (_data[usageIndex]._palIndex == palIndex) {
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
			int var2 = (palette[palIndex]._flags & 0x20) ||
				(((flags & 0x2000) || (palette[palIndex]._flags & 0x4000)) &&
				((flags & 0x1000) || (palCount == 0))) ? 0x7fff : 1;
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

	_vm->_palette->_rgbList[rgbIndex] = -1;

	return rgbIndex;
}


int PaletteUsage::rgbMerge(RGB6 &palEntry) {
	return ((palEntry.r + 1) / 4 - 1) * 38 + ((palEntry.g + 1) / 4 - 1) * 76 + 
		((palEntry.b + 1) / 4 - 1) * 14;
}

void PaletteUsage::transform(Common::Array<RGB6> &palette) {
	if (!empty()) {
		for (uint i = 0; i < _data.size(); ++i) {
			int palIndex = _data[i]._palIndex;
			_data[i] = palette[palIndex]._palIndex;
		}
	}
}

void PaletteUsage::updateUsage(Common::Array<int> &usageList, int sceneUsageIndex) {
	uint32 mask1 = 0xFFFFFFFF;
	uint32 mask2 = 0;

	for (uint idx = 0; idx < usageList.size(); ++idx) {
		uint32 bitMask = 1 << usageList[idx];
		mask1 ^= bitMask;
		mask2 |= bitMask;
		_vm->_palette->_rgbList[usageList[idx]] = 0;
	}

	uint32 mask3 = 1 << sceneUsageIndex;

	for (uint idx = 0; idx < PALETTE_COUNT; ++idx) {
		uint32 mask = mask2 & _vm->_palette->_palFlags[idx];
		if (mask) {
			_vm->_palette->_palFlags[idx] = (_vm->_palette->_palFlags[idx] &
				mask1) | mask3;
		}
	}

	_vm->_palette->_rgbList[sceneUsageIndex] = -1;
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
	_mainPalette[palIndex * 3] = VGA_COLOR_TRANS(r);
	_mainPalette[palIndex * 3 + 1] = VGA_COLOR_TRANS(g);
	_mainPalette[palIndex * 3 + 2] = VGA_COLOR_TRANS(b);

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
	_rgbList[0] = _rgbList[1] = -1;

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

const uint32 DEFAULT_VGA_PALETTE[PALETTE_COUNT] = {
	0x000000, 0x0000a8, 0x00a800, 0x00a8a8, 0xa80000, 0xa800a8, 0xa85400, 0xa8a8a8,
	0x545454, 0x5454fc, 0x54fc54, 0x54fcfc, 0xfc5454, 0xfc54fc, 0xfcfc54, 0xfcfcfc,
	0x000000, 0x141414, 0x202020, 0x2c2c2c, 0x383838, 0x444444, 0x505050, 0x606060,
	0x707070, 0x808080, 0x909090, 0xa0a0a0, 0xb4b4b4, 0xc8c8c8, 0xe0e0e0, 0xfcfcfc,
	0x0000fc, 0x4000fc, 0x7c00fc, 0xbc00fc, 0xfc00fc, 0xfc00bc, 0xfc007c, 0xfc0040,
	0xfc0000, 0xfc4000, 0xfc7c00, 0xfcbc00, 0xfcfc00, 0xbcfc00, 0x7cfc00, 0x40fc00,
	0x00fc00, 0x00fc40, 0x00fc7c, 0x00fcbc, 0x00fcfc, 0x00bcfc, 0x007cfc, 0x0040fc,
	0x7c7cfc, 0x9c7cfc, 0xbc7cfc, 0xdc7cfc, 0xfc7cfc, 0xfc7cdc, 0xfc7cbc, 0xfc7c9c,
	0xfc7c7c, 0xfc9c7c, 0xfcbc7c, 0xfcdc7c, 0xfcfc7c, 0xdcfc7c, 0xbcfc7c, 0x9cfc7c,
	0x7cfc7c, 0x7cfc9c, 0x7cfcbc, 0x7cfcdc, 0x7cfcfc, 0x7cdcfc, 0x7cbcfc, 0x7c9cfc,
	0xb4b4fc, 0xc4b4fc, 0xd8b4fc, 0xe8b4fc, 0xfcb4fc, 0xfcb4e8, 0xfcb4d8, 0xfcb4c4,
	0xfcb4b4, 0xfcc4b4, 0xfcd8b4, 0xfce8b4, 0xfcfcb4, 0xe8fcb4, 0xd8fcb4, 0xc4fcb4,
	0xb4fcb4, 0xb4fcc4, 0xb4fcd8, 0xb4fce8, 0xb4fcfc, 0xb4e8fc, 0xb4d8fc, 0xb4c4fc,
	0x000070, 0x1c0070, 0x380070, 0x540070, 0x700070, 0x700054, 0x700038, 0x70001c,
	0x700000, 0x701c00, 0x703800, 0x705400, 0x707000, 0x547000, 0x387000, 0x1c7000,
	0x007000, 0x00701c, 0x007038, 0x007054, 0x007070, 0x005470, 0x003870, 0x001c70,
	0x383870, 0x443870, 0x543870, 0x603870, 0x703870, 0x703860, 0x703854, 0x703844,
	0x703838, 0x704438, 0x705438, 0x706038, 0x707038, 0x607038, 0x547038, 0x447038,
	0x387038, 0x387044, 0x387054, 0x387060, 0x387070, 0x386070, 0x385470, 0x384470,
	0x505070, 0x585070, 0x605070, 0x685070, 0x705070, 0x705068, 0x705060, 0x705058,
	0x705050, 0x705850, 0x706050, 0x706850, 0x707050, 0x687050, 0x607050, 0x587050,
	0x507050, 0x507058, 0x507060, 0x507068, 0x507070, 0x506870, 0x506070, 0x505870,
	0x000040, 0x100040, 0x200040, 0x300040, 0x400040, 0x400030, 0x400020, 0x400010,
	0x400000, 0x401000, 0x402000, 0x403000, 0x404000, 0x304000, 0x204000, 0x104000,
	0x004000, 0x004010, 0x004020, 0x004030, 0x004040, 0x003040, 0x002040, 0x001040,
	0x202040, 0x282040, 0x302040, 0x382040, 0x402040, 0x402038, 0x402030, 0x402028,
	0x402020, 0x402820, 0x403020, 0x403820, 0x404020, 0x384020, 0x304020, 0x284020,
	0x204020, 0x204028, 0x204030, 0x204038, 0x204040, 0x203840, 0x203040, 0x202840,
	0x2c2c40, 0x302c40, 0x342c40, 0x3c2c40, 0x402c40, 0x402c3c, 0x402c34, 0x402c30,
	0x402c2c, 0x40302c, 0x40342c, 0x403c2c, 0x40402c, 0x3c402c, 0x34402c, 0x30402c,
	0x2c402c, 0x2c4030, 0x2c4034, 0x2c403c, 0x2c4040, 0x2c3c40, 0x2c3440, 0x2c3040,
	0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
};

void Palette::initVGAPalette(byte *palette) {
	byte *destP = palette;
	for (int palIndex = 0; palIndex < PALETTE_COUNT; ++palIndex) {
		for (int byteCtr = 2; byteCtr >= 0; --byteCtr)
			*destP++ = ((DEFAULT_VGA_PALETTE[palIndex] >> (8 * byteCtr)) & 0xff) >> 2;
	}
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

void Palette::sub7BBF8() {
	if ((_rgbList[31] != 0) && (_v1 == 0))
		error("Palette - Unexpected values");

	_v1 = -1;
	_rgbList[31] = -1;

	for (int i = 0; i < 256; i++) {
		if (_palFlags[i])
			_palFlags[i] |= 0x80000000;
	}
}

} // End of namespace MADS
