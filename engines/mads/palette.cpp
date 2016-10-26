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
#include "engines/util.h"
#include "graphics/palette.h"
#include "mads/mads.h"
#include "mads/msurface.h"
#include "mads/staticres.h"

namespace MADS {

#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)

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
	_data = nullptr;
}

void PaletteUsage::load(Common::Array<UsageEntry> *data) {
	_data = data;
}

void PaletteUsage::getKeyEntries(Common::Array<RGB6> &palette) {
	_data->clear();

	 for (uint i = 0; i < palette.size(); ++i) {
		 byte *uPtr = &palette[i]._flags;
		 if ((*uPtr & 0x10) && _data->size() < 3) {
			 _data->push_back(UsageEntry(i));
		 }
	 }
}

static bool sortHelper(const PaletteUsage::UsageEntry &ue1, const PaletteUsage::UsageEntry &ue2) {
	return ue1._sortValue < ue2._sortValue;
}

void PaletteUsage::prioritize(Common::Array<RGB6> &palette) {
	for (uint i = 0; i < _data->size(); ++i) {
		RGB6 &palEntry = palette[(*_data)[i]._palIndex];
		(*_data)[i]._sortValue = _vm->_palette->rgbMerge(palEntry);
	}

	Common::sort(_data->begin(), _data->end(), sortHelper);
}

static bool rangeSorter(const PaletteUsage::UsageRange &ur1, const PaletteUsage::UsageRange &ur2) {
	return ur1._v2 < ur2._v2;
}

int PaletteUsage::process(Common::Array<RGB6> &palette, uint flags) {
	int palLow;
	int palHigh = (flags & 0x800) ? 0x100 : 0xFC;
	int palIdx;

	PaletteUsage tempUsage(_vm);
	Common::Array<UsageEntry> tempUsageData;
	tempUsage.load(&tempUsageData);

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
	bool noUsageFlag = flags & 0x8000;
	bool hasUsage = _data != nullptr;
	bool flag1 = false;

	if (hasUsage) {
		if (noUsageFlag || _data->size() == 0)
			hasUsage = false;

		if (noUsageFlag && _data->size() > 0)
			flag1 = true;
	}

	if (hasUsage) {
		tempUsage.getKeyEntries(palette);
		tempUsage.prioritize(palette);
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
		bool changed = false;
		int newPalIndex = 0xFF;
		int v1 = palRange[palIndex]._v2;

		if (palette[v1]._flags & 8) {
			changed = true;
			newPalIndex = 0xFD;
		}

		if (hasUsage && palette[v1]._flags & 0x10) {
			for (uint usageIndex = 0; usageIndex < tempUsage._data->size() && !changed; ++usageIndex) {
				if ((*tempUsage._data)[usageIndex]._palIndex == palIndex) {
					changed = true;
					int dataIndex = MIN(usageIndex, _data->size() - 1);
					newPalIndex = (*_data)[dataIndex]._palIndex;
				}
			}
		}

		if (flag1 && palette[palIndex]._flags & 0x10) {
			for (uint usageIndex = 0; usageIndex < _data->size() && !changed; ++usageIndex) {
				if ((*_data)[usageIndex]._palIndex == palIndex) {
					changed = true;
					newPalIndex = 0xF0 + usageIndex;

					// Copy data into the high end of the main palette
					RGB6 &pSrc = palette[palIndex];
					byte *pDest = &_vm->_palette->_mainPalette[newPalIndex * 3];
					pDest[0] = pSrc.r;
					pDest[1] = pSrc.g;
					pDest[2] = pSrc.b;
				}
			}
		}

		if (!changed && !noUsageFlag) {
			int bestHash = (palette[palIndex]._flags & 0x20) ||
				(((flags & 0x2000) || (palette[palIndex]._flags & 0x40)) &&
				((flags & 0x1000) || (palCount == 0))) ? 0x7fff : 1;
			int var36 = (palette[palIndex]._flags & 0x80) ? 0 : 2;

			for (int idx = palLow; idx < palIdx; ++idx) {
				uint32 v = _vm->_palette->_palFlags[idx];
				if ((v & var3A) && !(v & var36)) {
					int hash;
					if (bestHash > 1) {
						hash = rgbFactor(&_vm->_palette->_mainPalette[idx * 3], palette[palIndex]);
					} else if (_vm->_palette->_mainPalette[idx * 3] != palette[palIndex].r ||
							_vm->_palette->_mainPalette[idx * 3 + 1] != palette[palIndex].g ||
							_vm->_palette->_mainPalette[idx * 3 + 2] != palette[palIndex].b) {
						hash = 1;
					} else {
						hash = 0;
					}

					if (bestHash > hash) {
						changed = true;
						newPalIndex = idx;
						bestHash = hash;
					}
				}
			}
		}

		if (!changed && (!(flags & 0x1000) || (!(palette[palIndex]._flags & 0x60) && !(flags & 0x2000)))) {
			for (int idx = freeIndex; idx < palIdx && !changed; ++idx) {
				if (!_vm->_palette->_palFlags[idx]) {
					--palCount;
					++freeIndex;
					changed = true;
					newPalIndex = idx;

					RGB6 &pSrc = palette[palIndex];
					byte *pDest = &_vm->_palette->_mainPalette[idx * 3];
					pDest[0] = pSrc.r;
					pDest[1] = pSrc.g;
					pDest[2] = pSrc.b;
				}
			}
		}

		// TODO: Not sure if it's valid or not for changed flag to ever be false.
		// In at least scene 318, when the doctor knocks you with the blackjack,
		// the changed flag can be false
		//assert(changed);

		// CHECKME: When pressing on F1 in the first screen, newPalIndex is set to 0xFF at this point
		// which is a valid value for the index. Maybe a better check would be "< 256" ?
		//assert(newPalIndex != -1);

		int var52 = (noUsageFlag && palette[palIndex]._u2) ? 2 : 0;

		_vm->_palette->_palFlags[newPalIndex] |= var52 | rgbMask;
		palette[palIndex]._palIndex = newPalIndex;
	}

	_vm->_palette->_rgbList[rgbIndex] = true;

	return rgbIndex;
}

void PaletteUsage::transform(Common::Array<RGB6> &palette) {
	if (!empty()) {
		for (uint i = 0; i < _data->size(); ++i) {
			int palIndex = (*_data)[i]._palIndex;
			(*_data)[i]._palIndex = palette[palIndex]._palIndex;
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
		_vm->_palette->_rgbList[usageList[idx]] = false;
	}

	uint32 mask3 = 1 << sceneUsageIndex;

	for (uint idx = 0; idx < PALETTE_COUNT; ++idx) {
		uint32 mask = mask2 & _vm->_palette->_palFlags[idx];
		if (mask) {
			_vm->_palette->_palFlags[idx] = (_vm->_palette->_palFlags[idx] &
				mask1) | mask3;
		}
	}

	_vm->_palette->_rgbList[sceneUsageIndex] = true;
}

void PaletteUsage::resetPalFlags(int idx) {
	if (idx >= 0 && idx < 32) {
		uint32 rgbMask = ~(1 << idx);

		uint32 *flagP = _vm->_palette->_palFlags;
		for (int i = 0; i < 256; ++i, ++flagP) {
			*flagP &= rgbMask;
			if (*flagP == 2)
				*flagP = 0;
		}

		_vm->_palette->_rgbList[idx] = false;
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

int PaletteUsage::checkRGB(const byte *rgb, int palStart, bool flag, int *palIndex) {
	Palette &palette = *_vm->_palette;
	bool match = false;
	int result;
	if (palStart >= 0) {
		result = palStart;
	} else {
		result = -1;
		for (int i = 0; i < palette._highRange; ++i) {
			if (!palette._rgbList[i]) {
				result = i;
				break;
			}
		}
	}

	if (result >= 0) {
		int mask = 1 << result;
		byte *palP = &palette._mainPalette[0];
		uint32 *flagsP = &palette._palFlags[0];

		for (; flagsP < &palette._palFlags[PALETTE_COUNT]; ++flagsP, ++result) {
			if ((!(*flagsP & 1) || flag) && !(*flagsP & 2)) {
				if (!memcmp(palP, rgb, 3)) {
					*flagsP |= mask;

					if (palIndex)
						*palIndex = result;
					match = true;
					break;
				}
			}
		}

		if (!match) {
			palP = &palette._mainPalette[0];
			flagsP = &palette._palFlags[0];

			for (int i = 0; i < PALETTE_COUNT; ++i, palP += 3, ++flagsP) {
				if (!*flagsP) {
					Common::copy(rgb, rgb + 3, palP);
					*flagsP |= mask;

					if (palIndex)
						*palIndex = i;
					match = true;
					break;
				}
			}
		}
	}

	assert(match);
	return result;
}

/*------------------------------------------------------------------------*/

void RGBList::clear() {
	for (int i = 0; i < 32; i++)
		_data[i] = false;
}

void RGBList::reset() {
	for (int i = 2; i < 32; i++)
		_data[i] = false;
}

int RGBList::scan() {
	for (int i = 0; i < 32; ++i) {
		if (!_data[i])
			return i;
	}

	error("RGBList was full");
}

void RGBList::copy(RGBList &src) {
	Common::copy(&src._data[0], &src._data[32], &_data[0]);
}

/*------------------------------------------------------------------------*/

Fader::Fader(MADSEngine *vm)
	: _vm(vm) {
	_colorFlags[0] = _colorFlags[1] = _colorFlags[2] = true;
	_colorFlags[3] = false;
	_colorValues[0] = _colorValues[1] = 0;
	_colorValues[2] = _colorValues[3] = 0;

	// TODO: It would be better if the fader routines could be refactored
	// to work directly with 8-bit RGB values rather than 6-bit RGB values
	Common::fill(&_rgb64Map[0], &_rgb64Map[PALETTE_COUNT], 0);
	for (int i = 0; i < 64; ++i)
		_rgb64Map[VGA_COLOR_TRANS(i)] = i;
	byte v = 0;
	for (int i = 0; i < PALETTE_COUNT; ++i) {
		if (_rgb64Map[i])
			v = _rgb64Map[i];
		else
			_rgb64Map[i] = v;
	}
}


void Fader::setPalette(const byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->setPalette(colors, start, num);
}

void Fader::grabPalette(byte *colors, uint start, uint num) {
	g_system->getPaletteManager()->grabPalette(colors, start, num);
}

void Fader::getFullPalette(byte palette[PALETTE_SIZE]) {
	grabPalette(&palette[0], 0, PALETTE_COUNT);
}

void Fader::setFullPalette(byte palette[PALETTE_SIZE]) {
	setPalette(&palette[0], 0, PALETTE_COUNT);
}

void Fader::fadeOut(byte palette[PALETTE_SIZE], byte *paletteMap,
		int baseColor, int numColors, int baseGrey, int numGreys,
		int tickDelay, int steps) {
	GreyEntry map[PALETTE_COUNT];
	int intensity;
	byte palIndex[PALETTE_COUNT][3];
	int8 signs[PALETTE_COUNT][3];

	mapToGreyRamp(palette, baseColor, numColors, baseGrey, numGreys, map);

	for (int palCtr = baseColor; palCtr < (baseColor + numColors); ++palCtr) {
		int index = palCtr - baseColor;
		for (int colorCtr = 0; colorCtr < 3; ++colorCtr) {
			if (_colorFlags[colorCtr]) {
				int shiftSign = _colorValues[colorCtr];
				if (shiftSign >= 0) {
					intensity = map[index]._intensity << shiftSign;
				} else {
					intensity = map[index]._intensity >> ABS(shiftSign);
				}
			} else {
				intensity = _colorValues[colorCtr];
			}

			int diff = intensity - _rgb64Map[palette[palCtr * 3 + colorCtr]];
			palIndex[palCtr][colorCtr] = (byte)ABS(diff);
			signs[palCtr][colorCtr] = (diff == 0) ? 0 : (diff < 0 ? -1 : 1);
		}
	}

	for (int stepCtr = 0; stepCtr < steps; ++stepCtr) {
		for (int palCtr = baseColor; palCtr < (baseColor + numColors); ++palCtr) {
			int index = palCtr - baseColor;
			for (int colorCtr = 0; colorCtr < 3; ++colorCtr) {
				map[index]._accum[colorCtr] += palIndex[palCtr][colorCtr];
				while (map[index]._accum[colorCtr] >= steps) {
					map[index]._accum[colorCtr] -= steps;

					byte rgb63 = _rgb64Map[palette[palCtr * 3 + colorCtr]] +
						signs[palCtr][colorCtr];
					palette[palCtr * 3 + colorCtr] = VGA_COLOR_TRANS(rgb63);
				}
			}
		}

		setFullPalette(palette);

		_vm->_events->waitForNextFrame();
	}

	if (paletteMap != nullptr) {
		for (int palCtr = 0; palCtr < numColors; palCtr++) {
			paletteMap[palCtr] = map[palCtr]._mapColor;
		}
	}
}

void Fader::fadeIn(byte palette[PALETTE_SIZE], byte destPalette[PALETTE_SIZE],
	int baseColor, int numColors, int baseGrey, int numGreys,
	int tickDelay, int steps) {
	GreyEntry map[PALETTE_COUNT];
	byte tempPal[PALETTE_SIZE];
	int8 signs[PALETTE_COUNT][3];
	byte palIndex[PALETTE_COUNT][3];
	int intensity;

	Common::copy(destPalette, destPalette + PALETTE_SIZE, tempPal);

	mapToGreyRamp(tempPal, baseColor, numColors, baseGrey, numGreys, map);

	for (int palCtr = baseColor; palCtr < (baseColor + numColors); ++palCtr) {
		int index = palCtr - baseColor;
		for (int colorCtr = 0; colorCtr < 3; ++colorCtr) {
			if (_colorFlags[colorCtr]) {
				int shiftSign = _colorValues[colorCtr];
				if (shiftSign >= 0)
					intensity = map[index]._intensity << shiftSign;
				else
					intensity = map[index]._intensity >> abs(shiftSign);
			} else
				intensity = _colorValues[colorCtr];

			int diff = _rgb64Map[destPalette[palCtr * 3 + colorCtr]] - intensity;
			palIndex[palCtr][colorCtr] = (byte)ABS(diff);
			signs[palCtr][colorCtr] = (diff == 0) ? 0 : (diff < 0 ? -1 : 1);

			map[index]._accum[colorCtr] = 0;
		}
	}

	for (int stepCtr = 0; stepCtr < steps; ++stepCtr) {
		for (int palCtr = baseColor; palCtr < (baseColor + numColors); ++palCtr) {
			int index = palCtr - baseColor;
			for (int colorCtr = 0; colorCtr < 3; ++colorCtr) {
				map[index]._accum[colorCtr] += palIndex[palCtr][colorCtr];
				while (map[index]._accum[colorCtr] >= steps) {
					map[index]._accum[colorCtr] -= steps;

					byte rgb63 = _rgb64Map[palette[palCtr * 3 + colorCtr]] +
						signs[palCtr][colorCtr];
					palette[palCtr * 3 + colorCtr] = VGA_COLOR_TRANS(rgb63);
				}
			}
		}

		setFullPalette(palette);

		_vm->_events->waitForNextFrame();
	}
}

void Fader::mapToGreyRamp(byte palette[PALETTE_SIZE], int baseColor, int numColors,
		int baseGrey, int numGreys, GreyEntry *map) {
	byte greyList[PALETTE_COUNT];
	byte greyMapping[PALETTE_COUNT];
	byte greyTable[64];
	byte greyIntensity[64];
	int intensity, shiftSign;

	getGreyValues(palette, greyList, baseColor, numColors);
	greyPopularity(greyList, greyTable, numColors);

	for (int idx = 0; idx < numColors; ++idx) {
		greyMapping[idx] = idx;
		Common::fill(&map[idx]._accum[0], &map[idx]._accum[3], 0);
	}

	for (int idx = 0; idx < PALETTE_COUNT; ++idx) {
		map[idx]._mapColor = (byte)idx;
	}

	// Sort the mapping lists
	insertionSort(numColors, greyList, greyMapping);

	// Initialize state variables
	int greySum = 0;
	int greyScan = 0;
	int greyMark = 0;
	int greyColors = 0;
	int greyAccum = 0;
	int firstColor = 0;

	for (int greyCtr = 0; greyCtr < 64; ++greyCtr) {
		for (int idx = 0; idx < greyTable[greyCtr]; ++idx) {
			greySum += greyList[greyScan++];
			++greyColors;

			greyAccum += numGreys;
			while (greyAccum >= numColors) {
				greyAccum -= numColors;
				if (greyColors > 0) {
					greyIntensity[greyMark] = (byte)(greySum / greyColors);
				}

				for (int rescan = firstColor; rescan < greyScan; ++rescan) {
					map[greyMapping[rescan]]._intensity = greyIntensity[greyMark];
					map[greyMapping[rescan]]._mapColor = (byte)(greyMark + baseGrey);
				}

				firstColor = greyScan;
				greySum = 0;
				greyColors = 0;
				++greyMark;
			}
		}
	}

	// Set the palette range of greyscale values to be used
	byte *palP = &palette[baseGrey * 3];
	for (int greys = 0; greys < numGreys; ++greys) {
		for (int color = 0; color < 3; ++color) {
			if (_colorFlags[color]) {
				shiftSign = (byte)_colorValues[color];
				if (shiftSign >= 0) {
					intensity = greyIntensity[greys] << shiftSign;
				} else {
					intensity = greyIntensity[greys] >> abs(shiftSign);
				}
			} else {
				intensity = _colorValues[color];
			}
			*palP++ = VGA_COLOR_TRANS(intensity);
		}
	}
}

void Fader::getGreyValues(const byte palette[PALETTE_SIZE],
		byte greyList[PALETTE_COUNT], int baseColor, int numColors) {
	const byte *palP = &palette[baseColor * 3];

	for (int i = 0; i < numColors; ++i, palP += 3) {
		int v = rgbMerge(palP[0], palP[1], palP[2]);
		greyList[i] = v >> 7;
	}
}

void Fader::greyPopularity(const byte greyList[PALETTE_COUNT],
		byte greyTable[64], int numColors) {
	Common::fill(&greyTable[0], &greyTable[64], 0);
	for (int i = 0; i < numColors; ++i) {
		int idx = greyList[i];
		++greyTable[idx];
	}
}

void Fader::insertionSort(int size, byte *id, byte *value) {
	bool restartFlag;
	int endIndex = size - 1;

	do {
		restartFlag = false;
		if (endIndex <= 0)
			break;

		for (int arrIndex = 0; arrIndex < endIndex && !restartFlag; ++arrIndex) {
			byte *idP = id + arrIndex;
			byte *valueP = value + arrIndex;

			// Check whether the next index is out of order with the one following it
			if (*idP > *(idP + 1)) {
				// Found an incorrect ordering
				restartFlag = true;

				// Save id/value at current index
				byte savedId = *idP;
				byte savedValue = *valueP;

				int moveCount = size - arrIndex - 1;
				if (moveCount > 0) {
					Common::copy(idP + 1, idP + moveCount + 1, idP);
					Common::copy(valueP + 1, valueP + moveCount + 1, valueP);
				}

				// Scan for insert spot
				int idx = 0;
				if (endIndex > 0) {
					bool breakFlag = false;
					for (; idx <= endIndex - 1 && !breakFlag; ++idx) {
						breakFlag = savedId < id[idx];
					}
				}

				// Set up an insert point for entry
				moveCount = size - idx - 1;
				if (moveCount > 0) {
					Common::copy_backward(id + idx, id + idx + moveCount, id + idx + moveCount + 1);
					Common::copy_backward(value + idx, value + idx + moveCount, value + idx + moveCount + 1);
				}

				// Set shifted values at the new position
				id[idx] = savedId;
				value[idx] = savedValue;
			}
		}
	} while (restartFlag);
}

int Fader::rgbMerge(RGB6 &palEntry) {
	return rgbMerge(palEntry.r, palEntry.g, palEntry.b);
}

int Fader::rgbMerge(byte r, byte g, byte b) {
	return _rgb64Map[r] * 38 + _rgb64Map[g] * 76 + _rgb64Map[b] * 14;
}

/*------------------------------------------------------------------------*/

Palette::Palette(MADSEngine *vm) : Fader(vm), _paletteUsage(vm) {
	_lockFl = false;
	_lowRange = 0;
	_highRange = 0;
	Common::fill(&_mainPalette[0], &_mainPalette[PALETTE_SIZE], 0);
	Common::fill(&_palFlags[0], &_palFlags[PALETTE_COUNT], 0);
}

void Palette::setEntry(byte palIndex, byte r, byte g, byte b) {
	_mainPalette[palIndex * 3] = VGA_COLOR_TRANS(r);
	_mainPalette[palIndex * 3 + 1] = VGA_COLOR_TRANS(g);
	_mainPalette[palIndex * 3 + 2] = VGA_COLOR_TRANS(b);

	setPalette((const byte *)&_mainPalette[palIndex * 3], palIndex, 1);
}

uint8 Palette::palIndexFromRgb(byte r, byte g, byte b, byte *paletteData) {
	byte index = 0;
	int32 minDist = 0x7fffffff;
	byte palData[PALETTE_SIZE];

	if (paletteData == NULL) {
		g_system->getPaletteManager()->grabPalette(palData, 0, PALETTE_COUNT);
		paletteData = &palData[0];
	}

	for (int palIndex = 0; palIndex < PALETTE_COUNT; ++palIndex) {
		int Rdiff = r - paletteData[palIndex * 3];
		int Gdiff = g - paletteData[palIndex * 3 + 1];
		int Bdiff = b - paletteData[palIndex * 3 + 2];

		if (Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff < minDist) {
			minDist = Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff;
			index = (uint8)palIndex;
		}
	}

	return (uint8)index;
}

void Palette::setGradient(byte *palette, int start, int count, int rgbValue1, int rgbValue2) {
	int rgbCurrent = rgbValue2;
	int rgbDiff = -(rgbValue2 - rgbValue1);

	if (count >  0) {
		byte *pDest = palette + start * 3;
		int endVal = count - 1;
		int numLeft = count;

		int rgbCtr = 0;
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

	// Reserve the start of the palette for things like on-screen text
	if (lowRange) {
		Common::fill(&_palFlags[0], &_palFlags[lowRange], 1);
	}

	// Reserve the high end of the palette for dialog display
	if (highRange) {
		Common::fill(&_palFlags[256 - highRange], &_palFlags[256], 1);
	}

	_rgbList.clear();
	_rgbList[0] = _rgbList[1] = true;

	_lockFl = false;
	_lowRange = lowRange;
	_highRange = highRange;
}

void Palette::initPalette() {
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

	_lockFl = false;
	_rgbList.reset();
}

void Palette::initVGAPalette(byte *palette) {
	byte *destP = palette;
	for (int palIndex = 0; palIndex < 16; ++palIndex) {
		for (int byteCtr = 2; byteCtr >= 0; --byteCtr)
			*destP++ = ((DEFAULT_VGA_LOW_PALETTE[palIndex] >> (8 * byteCtr)) & 0xff) >> 2;
	}

	destP = &palette[0xF0 * 3];
	for (int palIndex = 0; palIndex < 16; ++palIndex) {
		for (int byteCtr = 2; byteCtr >= 0; --byteCtr)
			*destP++ = ((DEFAULT_VGA_HIGH_PALETTE[palIndex] >> (8 * byteCtr)) & 0xff) >> 2;
	}
}

void Palette::setLowRange() {
	_mainPalette[0] = _mainPalette[1] = _mainPalette[2] = VGA_COLOR_TRANS(0);
	_mainPalette[3] = _mainPalette[4] = _mainPalette[5] = VGA_COLOR_TRANS(0x15);
	_mainPalette[6] = _mainPalette[7] = _mainPalette[8] = VGA_COLOR_TRANS(0x2A);
	_mainPalette[9] = _mainPalette[10] = _mainPalette[11] = VGA_COLOR_TRANS(0x3F);
	_vm->_palette->setPalette(_mainPalette, 0, 4);
}

void Palette::setColorFlags(byte r, byte g, byte b) {
	_colorFlags[0] = r;
	_colorFlags[1] = g;
	_colorFlags[2] = b;
}

void Palette::setColorValues(byte r, byte g, byte b) {
	_colorValues[0] = r;
	_colorValues[1] = g;
	_colorValues[2] = b;
}

void Palette::lock() {
	if (_rgbList[31] && !_lockFl)
		error("Palette Lock - Unexpected values");

	_lockFl = true;
	_rgbList[31] = true;

	for (int i = 0; i < 256; i++) {
		if (_palFlags[i])
			_palFlags[i] |= 0x80000000;
	}
}

void Palette::unlock() {
	if (!_lockFl)
		return;

	for (int i = 0; i < 256; i++)
		_palFlags[i] &= 0x7FFFFFFF;

	_rgbList[31] = false;
	_lockFl = false;
}

void Palette::refreshSceneColors() {
	int val = 18;
	if (_vm->_game->_scene._cyclingActive)
		val += _vm->_game->_scene._totalCycleColors;

	setPalette(_mainPalette + (val * 3), val, 256 - val);
}

int Palette::closestColor(const byte *matchColor, const byte *refPalette,
		int paletteInc, int count) {
	int bestColor = 0;
	int bestDistance = 0x7fff;

	for (int idx = 0; idx < count; ++idx) {
		// Figure out figure for 'distance' between two colors
		int distance = 0;
		for (int rgbIdx = 0; rgbIdx < RGB_SIZE; ++rgbIdx) {
			int diff = refPalette[rgbIdx] - matchColor[rgbIdx];
			distance += diff * diff;
		}

		// If the given color is a closer match to our color, store the index
		if (distance <= bestDistance) {
			bestDistance = distance;
			bestColor = idx;
		}

		refPalette += paletteInc;
	}

	return bestColor;
}


} // End of namespace MADS
